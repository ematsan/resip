#include <iostream>
#include "RegRunner.hxx"
#include "RegThread.hxx"
#include "RegMySQL.hxx"
#include "RegDB.hxx"

#include "resip/stack/SipStack.hxx"
#include "rutil/FdPoll.hxx"
#include "rutil/DnsUtil.hxx"

using namespace resip;
using namespace std;
using namespace registrar;

RegRunner::RegRunner()
   : mRunning(false)
   , mRegConfig(0)
   , mSipStack(0)
   , mStackThread(0)
   , mBase(0)
{
  InfoLog(<<"RegRunner constructor");
}

RegRunner::~RegRunner()
{
   InfoLog(<<"RegRunner destructor");
   if(mRunning) shutdown();
}

bool
RegRunner::run(int argc, char** argv)
{
  InfoLog(<<"RegRunner run");

  // Parse command line and configuration file
  resip_assert(!mRegConfig);
  Data defaultConfigFilename("reg.config");
  try
  {
     mRegConfig = new RegConfig();
     mRegConfig->parseConfig(argc, argv, defaultConfigFilename);
  }
  catch(BaseException& ex)
  {
     ErrLog( << "Error parsing configuration: " << ex);
     return false;
  }

  //connection with bd
  try{
    Data dbserver = mRegConfig->getConfigData("DBServer", "localhost");
    Data dbuser = mRegConfig->getConfigData("DBUser", "registrar");
    Data dbpassword = mRegConfig->getConfigData("DBPassword", "");
    Data dbname = mRegConfig->getConfigData("DBName", "registrar");
    unsigned int dbport = mRegConfig->getConfigInt("DBPort", 3306);

    mBase = new RegMySQL(dbserver, dbuser, dbpassword, dbname, dbport);
  }
  catch(BaseException& ex)
  {
       cerr << "Error connection with mysql: " << ex << endl;
       return false;
  }

  // Create SipStack and associated objects
  resip_assert(!mSipStack);
  try
  {

    mSipStack = new SipStack();

    int udpPort = mRegConfig->getConfigInt("UDPPort", 5060);
    int tcpPort = mRegConfig->getConfigInt("TCPPort", 5060);

    std::vector<Data> ipAddresses;
    mRegConfig->getConfigValue("IPAddresses", ipAddresses);
    if (ipAddresses.empty())
    {
      ipAddresses.push_back(Data::Empty);
    }

    for (Data ipAddress: ipAddresses)
    {
      if (udpPort)
      {
          mSipStack->addTransport(UDP, udpPort, V4, StunDisabled, ipAddress);
      }
      if (tcpPort)
      {
          mSipStack->addTransport(TCP, tcpPort, V4, StunDisabled, ipAddress);
      }
    }

    resip_assert(!mStackThread);
    //read contact
    Data realm = mRegConfig->getConfigData("Realm", "localhost");
    std::vector<Data> configDomains;
    mRegConfig->getConfigValue("Domains", configDomains);
    if (configDomains.empty())
    {
      configDomains.push_back("localhost");
      configDomains.push_back("127.0.0.1");
    }

    mStackThread = new RegThread (*mSipStack, realm, mBase, configDomains);

    mSipStack->run();
    mStackThread->run();
  }
  catch(BaseException& ex)
  {
     ErrLog(<< "Error creating SipStack: " << ex);
     return false;
  }

  mRunning = true;
  return true;
}

void
RegRunner::shutdown()
{
  InfoLog(<<"RegRunner shutdown");
  if(!mRunning) return;
  mStackThread->shutdown();
  mSipStack->shutdownAndJoinThreads();
  mStackThread->join();
  mBase->shutdown();

  delete mStackThread; mStackThread = 0;
  delete mSipStack; mSipStack = 0;
  delete mRegConfig; mRegConfig = 0;
  delete mBase; mBase = 0;

  mRunning = false;
}

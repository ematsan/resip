#include <iostream>
#include "RegRunner.hxx"
#include "Registrar.hxx"
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
   , mRegConfig(nullptr)
   , mSipStack(nullptr)
   , mStackThread(nullptr)
   , mBase(nullptr)
{
  InfoLog(<<"RegRunner constructor");
}

RegRunner::~RegRunner()
{
   InfoLog(<<"RegRunner destructor");
   if (mStackThread != nullptr)
   {
      mStackThread->shutdown();
    }

   if (mSipStack != nullptr)
   {
     mSipStack->shutdownAndJoinThreads();
     delete mSipStack;
     mSipStack = nullptr;
   }
   if (mStackThread != nullptr)
   {
     mStackThread->join();
     delete mStackThread;
     mStackThread = nullptr;
   }

   if (mBase != nullptr)
   {
     mBase->shutdown();
     delete mBase;
     mBase = nullptr;
   }

   if (mRegConfig != nullptr)
   {
     delete mRegConfig;
     mRegConfig = nullptr;
   }
}

bool
RegRunner::run(int argc, char** argv)
{
  InfoLog(<<"RegRunner run");

  // Parse command line and configuration file
  if (mRegConfig != nullptr)
  {
     cerr << "RegConfig not null" << endl;
     return false;
   }
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

  if(mBase != nullptr)
    return false;
  //connection with bd
  try{
    Data dbserver = mRegConfig->getConfigData("DBServer", "localhost");
    Data dbuser = mRegConfig->getConfigData("DBUser", "registrar");
    Data dbpassword = mRegConfig->getConfigData("DBPassword", "");
    Data dbname = mRegConfig->getConfigData("DBName", "registrar");
    unsigned int dbport = mRegConfig->getConfigInt("DBPort", 3306);

    mBase = new RegMySQL(dbserver, dbuser, dbpassword, dbname, dbport);
    if (!mBase->getConnected())
       return false;
  }
  catch(std::exception const& ex)
  {
       cerr << "Error connection with mysql: " << ex.what() << endl;
       return false;
  }

  // Create SipStack and associated objects
  if (mSipStack != nullptr)
  {
    cerr << "SipStack not null" << endl;
    return false;
  }

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

    if (mStackThread != nullptr)
    {
        cerr << "StackThread not null" << endl;
        return false;
      }
    //read contact
    Data realm = mRegConfig->getConfigData("Realm", "localhost");
    std::vector<Data> configDomains;
    mRegConfig->getConfigValue("Domains", configDomains);
    if (configDomains.empty())
    {
      configDomains.push_back("localhost");
      configDomains.push_back("127.0.0.1");
    }

    mStackThread = new Registrar (*mSipStack, realm, mBase, configDomains);

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
  if(!mRunning)
      return;
  mStackThread->shutdown();
  mSipStack->shutdownAndJoinThreads();
  mStackThread->join();
  mBase->shutdown();

  delete mStackThread;
  mStackThread = nullptr;
  delete mSipStack;
  mSipStack = nullptr;
  delete mRegConfig;
  mRegConfig = nullptr;
  delete mBase;
  mBase = nullptr;

  mRunning = false;
}

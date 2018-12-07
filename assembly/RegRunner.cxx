#include <iostream>
#include "RegRunner.hxx"
#include "RegThread.hxx"
#include "RegMySQL.hxx"

#include "resip/stack/SipStack.hxx"
#include "resip/stack/EventStackThread.cxx"
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
  cout<<"RegRunner constructor"<<endl;
}

RegRunner::~RegRunner()
{
   cout<<"RegRunner destructor"<<endl;
   if(mRunning) shutdown();
}

bool
RegRunner::run(int argc, char** argv)
{
  cout<<"RegRunner run"<<endl;

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
     cerr << "Error parsing configuration: " << ex << std::endl;
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
    mSipStack = new SipStack();//0, dnsServers, mAsyncProcessHandler, false, 0, 0, mFdPollGrp);

    int udpPort = mRegConfig->getConfigInt("UDPPort", 5060);
    int tcpPort = mRegConfig->getConfigInt("TCPPort", 5060);
    Data ipAddress = mRegConfig->getConfigData("IPAddress", Data::Empty, true);

    if (udpPort)
        mSipStack->addTransport(UDP, udpPort, V4, StunDisabled, ipAddress);
    if (tcpPort)
        mSipStack->addTransport(TCP, tcpPort, V4, StunDisabled, ipAddress);

    resip_assert(!mStackThread);
    //read contact
    Data realm = mRegConfig->getConfigData("Realm", "localhost");
    mStackThread = new RegThread (*mSipStack, realm, mBase);

    mSipStack->run();
    mStackThread->run();
  }
  catch(BaseException& ex)
  {
     cerr << "Error creating SipStack: " << ex << endl;
     return false;
  }

  mRunning = true;
  return true;
}

void
RegRunner::shutdown()
{
  cout<<"RegRunner shutdown"<<endl;
  if(!mRunning) return;
  mStackThread->shutdown();
  // Wait for all threads to shutdown, and destroy objects
  mSipStack->shutdownAndJoinThreads();
  mStackThread->join();

  delete mStackThread; mStackThread = 0;
  delete mSipStack; mSipStack = 0;
  delete mRegConfig; mRegConfig = 0;
  delete mBase; mBase = 0;

  mRunning = false;
}

#include <iostream>
#include "RegRunner.hxx"
#include "RegThread.hxx"

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
   /*, mFdPollGrp(0)
   , mAsyncProcessHandler(0)*/
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

  /* //test mRegConfig
  int port = mRegConfig->getConfigInt("Port", 5080);
  cout<<port<<endl;*/
  // Create SipStack and associated objects
  resip_assert(!mSipStack);
  try
  {
    // Create EventThreadInterruptor used to wake up the stack for
    // for reasons other than an Fd signalling
    /*resip_assert(!mFdPollGrp);
    mFdPollGrp = FdPollGrp::create();
    resip_assert(!mAsyncProcessHandler);
    mAsyncProcessHandler = new EventThreadInterruptor(*mFdPollGrp);
    DnsStub::NameserverList dnsServers = DnsStub::EmptyNameserverList;*/

    mSipStack = new SipStack();//0, dnsServers, mAsyncProcessHandler, false, 0, 0, mFdPollGrp);

  //  Data ipAddress = mRegConfig->getConfigData("IPAddress", Data::Empty, true);
    /*bool isV4Address = DnsUtil::isIpV4Address(ipAddress);
  //  bool isV6Address = DnsUtil::isIpV6Address(ipAddress);
    if (!ipAddress.empty())
    {
       ErrLog(<< "Malformed IP-address found in IPAddress setting, ignoring (binding to all interfaces): " << ipAddress);
       return false;
    }*/
    int udpPort = mRegConfig->getConfigInt("UDPPort", 5060);
    int tcpPort = mRegConfig->getConfigInt("TCPPort", 5060);
    if (udpPort)
        mSipStack->addTransport(UDP, udpPort);
    if (tcpPort)
        mSipStack->addTransport(TCP, tcpPort);


    resip_assert(!mStackThread);
    //read contact
    Data realm = mRegConfig->getConfigData("Realm", "localhost");
    mStackThread = new RegThread (*mSipStack, realm);

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
/*  delete mAsyncProcessHandler; mAsyncProcessHandler = 0;
  delete mFdPollGrp; mFdPollGrp = 0;*/
  delete mRegConfig; mRegConfig = 0;

  mRunning = false;
}

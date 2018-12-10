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
    bool mUseV6, mUseV4;
    mUseV4 = !mRegConfig->getConfigBool("DisableIPv4", false);
 #ifdef USE_IPV6
    mUseV6 = mRegConfig->getConfigBool("EnableIPv6", true);
 #else
    bool useV6 = false;
 #endif
    if (mUseV4)
       InfoLog (<<"V4 enabled");
    if (mUseV6)
       InfoLog (<<"V6 enabled");
    // Build DNS Server list from config
    DnsStub::NameserverList dnsServers;
    std::vector<resip::Data> dnsServersConfig;
    mRegConfig->getConfigValue("DNSServers", dnsServersConfig);
    for(std::vector<resip::Data>::iterator it = dnsServersConfig.begin(); it != dnsServersConfig.end(); it++)
    {
       if((mUseV4 && DnsUtil::isIpV4Address(*it)) || (mUseV6 && DnsUtil::isIpV6Address(*it)))
       {
          InfoLog(<< "Using DNS Server from config: " << *it);
          dnsServers.push_back(Tuple(*it, 0, UNKNOWN_TRANSPORT).toGenericIPAddress());
       }
    }


    mSipStack = new SipStack(0, dnsServers);//, mAsyncProcessHandler, false, 0, 0, mFdPollGrp);

    int udpPort = mRegConfig->getConfigInt("UDPPort", 5060);
    int tcpPort = mRegConfig->getConfigInt("TCPPort", 5060);
    Data ipAddress = mRegConfig->getConfigData("IPAddress", Data::Empty, true);

    if (udpPort)
    {
        mSipStack->addTransport(UDP, udpPort, V4, StunDisabled, ipAddress);
        /*if (mUseV6)
          mSipStack->addTransport(UDP, udpPort, V6, StunDisabled, ipAddress);*/
      }
    if (tcpPort)
    {
        mSipStack->addTransport(TCP, tcpPort, V4, StunDisabled, ipAddress);
        /*if (mUseV6)
          mSipStack->addTransport(TCP, tcpPort, V6, StunDisabled, ipAddress);*/
      }

      /*  int tlsPort = mRegConfig->getConfigInt("TLSPort", 5061);
        int wsPort = mRegConfig->getConfigInt("WSPort", 80);
        int wssPort = mRegConfig->getConfigInt("WSSPort", 443);
        int dtlsPort = mRegConfig->getConfigInt("DTLSPort", 0);
        mSipStack->addTransport(DTLS, dtlsPort, V4, StunDisabled, ipAddress);
        mSipStack->addTransport(TLS, tlsPort, V4, StunDisabled, ipAddress);
        mSipStack->addTransport(WS, wsPort, V4, StunDisabled, ipAddress);
        mSipStack->addTransport(WSS, wssPort, V4, StunDisabled, ipAddress);*/
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

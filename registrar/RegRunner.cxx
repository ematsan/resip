#include <iostream>
#include "RegRunner.hxx"
#include "RegConfig.hxx"


#include "resip/stack/ssl/Security.hxx"
#include "resip/stack/Compression.hxx"
#include "resip/stack/EventStackThread.cxx"

#include "rutil/FdPoll.hxx"
#include "rutil/AsyncProcessHandler.hxx"
#include "rutil/DnsUtil.hxx"
#include "rutil/GeneralCongestionManager.hxx"

using namespace resip;
using namespace std;

using namespace registrar;


RegRunner::RegRunner()
   : mRunning(false)
   , mRegConfig(0)
   , mUseV4(true)
   , mUseV6 (false)
   , mFdPollGrp(0)
   , mAsyncProcessHandler(0)
   , mThreadedStack(false)
   , mSipStack(0)
   , mStackThread(0)
   , mCongestionManager(0)
{
  //cout<<"RegRunner start"<<endl;
}

RegRunner::~RegRunner()
{
   //cout<<"RegRunner stop"<<endl;
   if(mRunning) shutdown();
}

bool
RegRunner::run(int argc, char** argv)
{
  if(mRunning) return false;

 // Store original arc and argv - so we can reuse them on restart request
  mArgc = argc;
  mArgv = argv;


  // Parse command line and configuration file
  resip_assert(!mRegConfig);
  Data defaultConfigFilename("reg.config");
  try
  {
     mRegConfig = new RegConfig(); //RegistrarProxyConfig();//ReproRunner class
     mRegConfig->parseConfig(mArgc, mArgv, defaultConfigFilename);
  }
  catch(BaseException& ex)
    {
       std::cerr << "Error parsing configuration: " << ex << std::endl;
       return false;
    }



    // Create SipStack and associated objects
    if(!createSipStack())
    {
       return false;
    }


/*
// Load the plugins after creating the stack, as they may need it
if(!loadPlugins())
{
   return false;
}

// Drop privileges (can do this now that sockets are bound)
Data runAsUser = mRegConfig->getConfigData("RunAsUser", Data::Empty, true);
Data runAsGroup = mRegConfig->getConfigData("RunAsGroup", Data::Empty, true);
if(!runAsUser.empty())
{
   InfoLog( << "Trying to drop privileges, configured uid = " << runAsUser << " gid = " << runAsGroup);
   dropPrivileges(runAsUser, runAsGroup);
}

// Create datastore
if(!createDatastore())
{
   return false;
}

// Create authentication mechanism
createAuthenticatorFactory();

// Create DialogUsageManager that handles ServerRegistration,
// and potentially certificate subscription server
createDialogUsageManager();

// Create the Proxy and associate objects
if(!createProxy())
{
   return false;
}

// Create HTTP WebAdmin and Thread
if(!createWebAdmin())
{
   return false;
}

// Create reg sync components if required
createRegSync();

// Create command server if required
if(!mRestarting)
{
   createCommandServer();
}*/


// Make it all go - startup all threads
mThreadedStack = mRegConfig->getConfigBool("ThreadedStack", true);
if(mThreadedStack)
{
   // If configured, then start the sub-threads within the stack
   mSipStack->run();
}
mStackThread->run();


/*if(mDumThread)
{
   mDumThread->run();
}
mProxy->run();
if(mWebAdminThread)
{
   mWebAdminThread->run();
}
if(!mRestarting && mCommandServerThread)
{
   mCommandServerThread->run();
}
if(mRegSyncServerThread)
{
   mRegSyncServerThread->run();
}
if(mRegSyncClient)
{
   mRegSyncClient->run();
}
*/

    mRunning = true;
    return true;
}

void
RegRunner::shutdown()
{
  //cout<<"Shutdown"<<endl;
  cout<<endl;
  //InfoLog (<< "Shutdown RegRunner");
  if(!mRunning) return;
  mStackThread->shutdown();
  // Wait for all threads to shutdown, and destroy objects
  //mProxy->join();
  if(mThreadedStack)
  {
     mSipStack->shutdownAndJoinThreads();
  }
  mStackThread->join();
  mSipStack->setCongestionManager(0);
  cleanupObjects();
  mRunning = false;
  mThreadedStack = false;
}

void
RegRunner::cleanupObjects()
{
//   if(!mRestarting)
   //{
      // We leave command server running during restart
//      delete mCommandServerThread; mCommandServerThread = 0;
//      for(std::list<CommandServer*>::iterator it = mCommandServerList.begin(); it != mCommandServerList.end(); it++)
//      {
//         delete (*it);
//      }
//      mCommandServerList.clear();
//   }
//   delete mRegSyncServerThread; mRegSyncServerThread = 0;
//   delete mRegSyncServerV6; mRegSyncServerV6 = 0;
//   delete mRegSyncServerV4; mRegSyncServerV4 = 0;
//   delete mRegSyncClient; mRegSyncClient = 0;
//#if defined(USE_SSL)
//   delete mCertServer; mCertServer = 0;
//#endif
   //delete mDumThread; mDumThread = 0;
   //delete mDum; mDum = 0;
   //delete mRegistrar; mRegistrar = 0;
   //delete mPresenceServer; mPresenceServer = 0;
   //delete mWebAdminThread; mWebAdminThread = 0;
   //for(std::list<WebAdmin*>::iterator it = mWebAdminList.begin(); it != mWebAdminList.end(); it++)
   //{
  //    delete (*it);
  // }
   //mWebAdminList.clear();
   //delete mProxy; mProxy = 0;
   //delete mBaboons; mBaboons = 0;
   //delete mLemurs; mLemurs = 0;
   //delete mMonkeys; mMonkeys = 0;
   //delete mAuthFactory; mAuthFactory = 0;
   //delete mAsyncProcessorDispatcher; mAsyncProcessorDispatcher = 0;
   //if(!mRestarting)
   //{
      // If we are restarting then leave the In Memory Registration and Publication database intact
  //    delete mRegistrationPersistenceManager; mRegistrationPersistenceManager = 0;
  //    delete mPublicationPersistenceManager; mPublicationPersistenceManager = 0;
   //}
   //delete mAbstractDb; mAbstractDb = 0;
   //delete mRuntimeAbstractDb; mRuntimeAbstractDb = 0;
   delete mStackThread; mStackThread = 0;
   delete mSipStack; mSipStack = 0;
   delete mCongestionManager; mCongestionManager = 0;
   delete mAsyncProcessHandler; mAsyncProcessHandler = 0;
   delete mFdPollGrp; mFdPollGrp = 0;
   delete mRegConfig; mRegConfig = 0;
}


bool
RegRunner::createSipStack()
{
   // Override T1 timer if configured to do so
  /* unsigned long overrideT1 = mRegConfig->getConfigInt("TimerT1", 0);
   if(overrideT1)
   {
      WarningLog(<< "Overriding T1! (new value is " << overrideT1 << ")");
      resip::Timer::resetT1(overrideT1);
   }

   // Set TCP Connect timeout
   resip::Timer::TcpConnectTimeout = mRegConfig->getConfigInt("TCPConnectTimeout", 0);

   unsigned long messageSizeLimit = mRegConfig->getConfigUnsignedLong("StreamMessageSizeLimit", 0);
   if(messageSizeLimit > 0)
   {
      DebugLog(<< "Using maximum message size "<< messageSizeLimit << " on stream-based transports");
      ConnectionBase::setMessageSizeMax(messageSizeLimit);
   }*/

   // Create Security (TLS / Certificates) and Compression (SigComp) objects if
   // pre-precessor defines are enabled
   Security* security = 0;
   Compression* compression = 0;
/*#ifdef USE_SSL
   setOpenSSLCTXOptionsFromConfig(
         "OpenSSLCTXSetOptions", BaseSecurity::OpenSSLCTXSetOptions);
   setOpenSSLCTXOptionsFromConfig(
         "OpenSSLCTXClearOptions", BaseSecurity::OpenSSLCTXClearOptions);
   Security::CipherList cipherList = Security::StrongestSuite;
   Data ciphers = mRegConfig->getConfigData("OpenSSLCipherList", Data::Empty);
   if(!ciphers.empty())
   {
      cipherList = ciphers;
   }
   Data certPath = mRegConfig->getConfigData("CertificatePath", Data::Empty);
   Data dHParamsFilename = mRegConfig->getConfigData("TlsDHParamsFilename", Data::Empty);
   if(certPath.empty())
   {
      security = new Security(cipherList, mRegConfig->getConfigData("TLSPrivateKeyPassPhrase", Data::Empty), dHParamsFilename);
   }
   else
   {
      security = new Security(certPath, cipherList, mRegConfig->getConfigData("TLSPrivateKeyPassPhrase", Data::Empty), dHParamsFilename);
   }
   Data caDir;
   mRegConfig->getConfigValue("CADirectory", caDir);
   if(!caDir.empty())
   {
      security->addCADirectory(caDir);
   }
   Data caFile;
   mRegConfig->getConfigValue("CAFile", caFile);
   if(!caFile.empty())
   {
      security->addCAFile(caFile);
   }
#endif

#ifdef USE_SIGCOMP
   compression = new Compression(Compression::DEFLATE);
#endif
*/
   // Create EventThreadInterruptor used to wake up the stack for
   // for reasons other than an Fd signalling
   resip_assert(!mFdPollGrp);
   mFdPollGrp = FdPollGrp::create();
   resip_assert(!mAsyncProcessHandler);
   mAsyncProcessHandler = new EventThreadInterruptor(*mFdPollGrp);

   // Set Flags that will enable/disable IPv4 and/or IPv6, based on
   // configuration and pre-processor flags
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

   // Create the SipStack Object
   resip_assert(!mSipStack);
   mSipStack = new SipStack(security,
                            dnsServers,
                            mAsyncProcessHandler,
                            false,
                            0,
                            compression,
                            mFdPollGrp);
//cout<<"\n\n\n\n\n\nTETETETETET\n\n\n\n\n\n\n"<<endl;
   // Set any enum suffixes from configuration
  /* std::vector<Data> enumSuffixes;
   mRegConfig->getConfigValue("EnumSuffixes", enumSuffixes);
   if (enumSuffixes.size() > 0)
   {
      mSipStack->setEnumSuffixes(enumSuffixes);
   }

   // Set any enum domains from configuration
   std::map<Data,Data> enumDomains;
   std::vector<Data> _enumDomains;
   mRegConfig->getConfigValue("EnumDomains", _enumDomains);
   if (enumSuffixes.size() > 0)
   {
      for(std::vector<Data>::iterator it = _enumDomains.begin(); it != _enumDomains.end(); it++)
      {
         enumDomains[*it] = *it;
      }
      mSipStack->setEnumDomains(enumDomains);
   }

   // Add External Stats handler
   mSipStack->setExternalStatsHandler(this);

   // Set Transport SipMessage Logging Handler - if enabled
   if(mRegConfig->getConfigBool("EnableSipMessageLogging", false))
   {
       mSipStack->setTransportSipMessageLoggingHandler(SharedPtr<ReproSipMessageLoggingHandler>(new ReproSipMessageLoggingHandler));
   }
*/
   // Add stack transports
   bool allTransportsSpecifyRecordRoute=false;
   if(!addTransports(allTransportsSpecifyRecordRoute))
   {
      cleanupObjects();
      return false;
   }
/*
   // Enable and configure RFC5626 Outbound support
   InteropHelper::setOutboundVersion(mRegConfig->getConfigInt("OutboundVersion", 5626));
   InteropHelper::setOutboundSupported(mRegConfig->getConfigBool("DisableOutbound", false) ? false : true);
   InteropHelper::setRRTokenHackEnabled(mRegConfig->getConfigBool("EnableFlowTokens", false));
   InteropHelper::setAssumeFirstHopSupportsOutboundEnabled(mRegConfig->getConfigBool("AssumeFirstHopSupportsOutbound", false));
   Data clientNATDetectionMode = mRegConfig->getConfigData("ClientNatDetectionMode", "DISABLED");
   if(isEqualNoCase(clientNATDetectionMode, "ENABLED"))
   {
      InteropHelper::setClientNATDetectionMode(InteropHelper::ClientNATDetectionEnabled);
   }
   else if(isEqualNoCase(clientNATDetectionMode, "PRIVATE_TO_PUBLIC"))
   {
      InteropHelper::setClientNATDetectionMode(InteropHelper::ClientNATDetectionPrivateToPublicOnly);
   }
   ConnectionManager::MinimumGcHeadroom = mRegConfig->getConfigUnsignedLong("TCPMinimumGCHeadroom", 0);
   unsigned long tcpConnectionGCAge = mRegConfig->getConfigUnsignedLong("TCPConnectionGCAge", 0);
   if(tcpConnectionGCAge > 0)
   {
      ConnectionManager::MinimumGcAge = tcpConnectionGCAge * 1000;
      ConnectionManager::EnableAgressiveGc = true;
   }
   unsigned long outboundFlowTimer = mRegConfig->getConfigUnsignedLong("FlowTimer", 0);
   if(outboundFlowTimer > 0)
   {
      InteropHelper::setFlowTimerSeconds(outboundFlowTimer);
      if(tcpConnectionGCAge == 0)
      {
         // This should be set too when using outboundFlowTimer
         ConnectionManager::MinimumGcAge = 7200000;
      }
      ConnectionManager::EnableAgressiveGc = true;
   }

   // Check Path and RecordRoute settings, print warning if features are enabled that
   // require record-routing and record-route uri(s) is not configured
   bool assumePath = mRegConfig->getConfigBool("AssumePath", false);
   bool forceRecordRoute = mRegConfig->getConfigBool("ForceRecordRouting", false);
   Uri recordRouteUri;
   mRegConfig->getConfigValue("RecordRouteUri", recordRouteUri);
   if((InteropHelper::getOutboundSupported()
         || InteropHelper::getRRTokenHackEnabled()
         || InteropHelper::getClientNATDetectionMode() != InteropHelper::ClientNATDetectionDisabled
         || assumePath
         || forceRecordRoute
      )
      && !(allTransportsSpecifyRecordRoute || !recordRouteUri.host().empty()))
   {
      CritLog(<< "In order for outbound support, the Record-Route flow-token"
      " hack, or force-record-route to work, you MUST specify a Record-Route URI. Launching "
      "without...");
      InteropHelper::setOutboundSupported(false);
      InteropHelper::setRRTokenHackEnabled(false);
      InteropHelper::setClientNATDetectionMode(InteropHelper::ClientNATDetectionDisabled);
      assumePath = false;
      forceRecordRoute=false;
   }*/

   // Configure misc. stack settings
   mSipStack->setFixBadDialogIdentifiers(false);
   mSipStack->setFixBadCSeqNumbers(false);

   //Set statistic manager
   //int statsLogInterval = mRegConfig->getConfigInt("StatisticsLogInterval", 60); //3600 - config
   int statsLogInterval = 30;
   if(statsLogInterval > 0)
   {
      mSipStack->setStatisticsInterval(statsLogInterval);
      mSipStack->statisticsManagerEnabled() = true;
   }
   else
   {
      mSipStack->statisticsManagerEnabled() = false;
   }

   // Create Congestion Manager, if required
   resip_assert(!mCongestionManager);
   if(mRegConfig->getConfigBool("CongestionManagement", true))
   {
      Data metricData = mRegConfig->getConfigData("CongestionManagementMetric", "WAIT_TIME", true);
      GeneralCongestionManager::MetricType metric = GeneralCongestionManager::WAIT_TIME;
      if(isEqualNoCase(metricData, "TIME_DEPTH"))
      {
         metric = GeneralCongestionManager::TIME_DEPTH;
      }
      else if(isEqualNoCase(metricData, "SIZE"))
      {
         metric = GeneralCongestionManager::SIZE;
      }
      else if(!isEqualNoCase(metricData, "WAIT_TIME"))
      {
         WarningLog( << "CongestionManagementMetric specified as an unknown value (" << metricData << "), defaulting to WAIT_TIME.");
      }
      mCongestionManager = new GeneralCongestionManager(
                                          metric,
                                          mRegConfig->getConfigUnsignedLong("CongestionManagementTolerance", 200));
      mSipStack->setCongestionManager(mCongestionManager);
   }

   // Create base thread to run stack in (note:  stack may use other sub-threads, depending on configuration)
   resip_assert(!mStackThread);
   mStackThread = new EventStackThread(*mSipStack,
                                       *dynamic_cast<EventThreadInterruptor*>(mAsyncProcessHandler),
                                       *mFdPollGrp);
   return true;
}


bool
RegRunner::addTransports(bool& allTransportsSpecifyRecordRoute)
{
   resip_assert(mRegConfig);
   resip_assert(mSipStack);
   try
   {
       Data ipAddress = mRegConfig->getConfigData("IPAddress", Data::Empty, true);
       bool isV4Address = DnsUtil::isIpV4Address(ipAddress);
       bool isV6Address = DnsUtil::isIpV6Address(ipAddress);
       if(!isV4Address && !isV6Address)
         {
            if (!ipAddress.empty())
            {
               ErrLog(<< "Malformed IP-address found in IPAddress setting, ignoring (binding to all interfaces): " << ipAddress);
            }
            ipAddress = Data::Empty;
            isV4Address = true;
            isV6Address = true;
         }
         int udpPort = mRegConfig->getConfigInt("UDPPort", 5060);
         int tcpPort = mRegConfig->getConfigInt("TCPPort", 5060);

       if (udpPort)
         {
            if (mUseV4 && isV4Address) mSipStack->addTransport(UDP, udpPort, V4, StunEnabled, ipAddress);
            if (mUseV6 && isV6Address) mSipStack->addTransport(UDP, udpPort, V6, StunEnabled, ipAddress);
         }
       if (tcpPort)
         {
            if (mUseV4 && isV4Address) mSipStack->addTransport(TCP, tcpPort, V4, StunEnabled, ipAddress);
            if (mUseV6 && isV6Address) mSipStack->addTransport(TCP, tcpPort, V6, StunEnabled, ipAddress);
         }
   }
   catch (BaseException& e)
   {
      std::cerr << "Likely a port is already in use" << endl;
      InfoLog (<< "Caught: " << e);
      return false;
   }
   return true;
}

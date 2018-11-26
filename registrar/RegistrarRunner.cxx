#include <iostream>
#include "RegistrarRunner.hxx"
#include "RegConfig.hxx"
//#include "ProxyConfig.hxx"

/*//#include "repro/ProxyConfig.hxx"
#include "repro/MySqlDb.hxx"
#include "repro/AbstractDb.hxx"
#include "repro/RegSyncClient.hxx"
#include "repro/RegSyncServer.hxx"
#include "repro/RegSyncServerThread.hxx"

#include <rutil/ConfigParse.hxx>
#include <rutil/GeneralCongestionManager.hxx>

#include "rutil/BaseException.hxx"
#include "rutil/Log.hxx"
#include "rutil/Logger.hxx"
#include "rutil/DnsUtil.hxx"

#include "resip/stack/EventStackThread.hxx"
#include "resip/stack/SipStack.hxx"*/



using namespace resip;
using namespace repro;
using namespace std;


RegistrarRunner::RegistrarRunner()
   : mRunning(false)
   , mThreadedStack(false)
   , mProxyConfig(0)
   , mFdPollGrp(0)
   , mUseV4(true)
   , mUseV6 (false)
   , mAsyncProcessHandler(0)
   , mSipStack(0)
   , mStackThread(0)
   , mAbstractDb(0)
   , mRuntimeAbstractDb(0)
   , mRegSyncClient(0)
   , mRegSyncServerV4(0)
   , mRegSyncServerThread(0)
   , mCongestionManager(0)
{
  cout<<"start"<<endl;
}

RegistrarRunner::~RegistrarRunner()
{
   cout<<"shutdown"<<endl;
   if(mRunning) shutdown();
}

bool
RegistrarRunner::run(int argc, char** argv)
{
  if(mRunning) return false;

 // Store original arc and argv - so we can reuse them on restart request
  mArgc = argc;
  mArgv = argv;


  // Parse command line and configuration file
  resip_assert(!mProxyConfig);
  Data defaultConfigFilename("registrar.config");
  try
  {
     //cout<<"parse"<<endl;
     //mProxyConfig = new ConfigParse();//RegistrarProxyConfig();//ReproRunner class
     mRegConfig = new RegConfig(); //RegistrarProxyConfig();//ReproRunner class
     mRegConfig->parseConfig(mArgc, mArgv, defaultConfigFilename);
  }
/*  catch(BaseException& ex)
  {
     std::cerr << "Error parsing configuration: " << ex << std::endl;
     return false;
  }


    //cout<<"Create SipStack and associated objects"<<endl;
    if(!createSipStack())
    {
       return false;
    }*/

    // Load the plugins after creating the stack, as they may need it
  /*  if(!loadPlugins())
    {
       return false;
    }

    // Drop privileges (can do this now that sockets are bound)
    Data runAsUser = mProxyConfig->getConfigData("RunAsUser", Data::Empty, true);
    Data runAsGroup = mProxyConfig->getConfigData("RunAsGroup", Data::Empty, true);
    if(!runAsUser.empty())
    {
       InfoLog( << "Trying to drop privileges, configured uid = " << runAsUser << " gid = " << runAsGroup);
       dropPrivileges(runAsUser, runAsGroup);
    }*/

    // Create datastore
  /*  if(!createDatastore())
    {
       return false;
    }*/
/*
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
  /*  mThreadedStack = mProxyConfig->getConfigBool("ThreadedStack", true);
    if(mThreadedStack)
    {
       // If configured, then start the sub-threads within the stack
       mSipStack->run();
    }
    mStackThread->run();*/
  /* if(mDumThread)
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
    }*/
  /*  if(mRegSyncServerThread)
    {
       mRegSyncServerThread->run();
    }
    if(mRegSyncClient)
    {
       mRegSyncClient->run();
    }*/

/*  mRunning = true;
  cout<<"Start registrar"<<endl;
  return true;*/
}

void
RegistrarRunner::shutdown()
{
 //cout<<"shutdown"<<endl;
}

bool
RegistrarRunner::createSipStack()
{
   //cout<<"Create SipStack and associated objects"<<endl;
   // Override T1 timer if configured to do so
  /* unsigned long overrideT1 = mProxyConfig->getConfigInt("TimerT1", 0);
   if(overrideT1)
   {
      //WarningLog(<< "Overriding T1! (new value is " << overrideT1 << ")");
      resip::Timer::resetT1(overrideT1);
   }

   // Set TCP Connect timeout
   resip::Timer::TcpConnectTimeout = mProxyConfig->getConfigInt("TCPConnectTimeout", 0);

   unsigned long messageSizeLimit = mProxyConfig->getConfigUnsignedLong("StreamMessageSizeLimit", 0);

   // Create Security (TLS / Certificates) and Compression (SigComp) objects if
   // pre-precessor defines are enabled
   Security* security = 0;
   Compression* compression = 0;

   // Create EventThreadInterruptor used to wake up the stack for
   // for reasons other than an Fd signalling
   resip_assert(!mFdPollGrp);
   mFdPollGrp = FdPollGrp::create();
   resip_assert(!mAsyncProcessHandler);
   mAsyncProcessHandler = new EventThreadInterruptor(*mFdPollGrp);

   // Set Flags that will enable/disable IPv4 and/or IPv6, based on
   // configuration and pre-processor flags
   mUseV4 = !mProxyConfig->getConfigBool("DisableIPv4", false);
#ifdef USE_IPV6
   mUseV6 = mProxyConfig->getConfigBool("EnableIPv6", true);
#else
   bool useV6 = false;
#endif
   if (mUseV4) cout << "V4 enabled" << endl;
   if (mUseV6) cout << "V6 enabled" << endl;



   // Build DNS Server list from config
   DnsStub::NameserverList dnsServers;
   std::vector<resip::Data> dnsServersConfig;
   mProxyConfig->getConfigValue("DNSServers", dnsServersConfig);
   for(std::vector<resip::Data>::iterator it = dnsServersConfig.begin(); it != dnsServersConfig.end(); it++)
   {
      if(mUseV4 && DnsUtil::isIpV4Address(*it))
      {
         cout<< "Using DNS Server from config: " << *it<<endl;
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



   // Add External Stats handler
  // mSipStack->setExternalStatsHandler(this);

  /*

   // Add stack transports
   bool allTransportsSpecifyRecordRoute=false;
   if(!addTransports(allTransportsSpecifyRecordRoute))
   {
      cleanupObjects();
      return false;
   }

   // Enable and configure RFC5626 Outbound support
   InteropHelper::setOutboundVersion(mProxyConfig->getConfigInt("OutboundVersion", 5626));
   InteropHelper::setOutboundSupported(mProxyConfig->getConfigBool("DisableOutbound", false) ? false : true);
   InteropHelper::setRRTokenHackEnabled(mProxyConfig->getConfigBool("EnableFlowTokens", false));
   InteropHelper::setAssumeFirstHopSupportsOutboundEnabled(mProxyConfig->getConfigBool("AssumeFirstHopSupportsOutbound", false));
   Data clientNATDetectionMode = mProxyConfig->getConfigData("ClientNatDetectionMode", "DISABLED");
   if(isEqualNoCase(clientNATDetectionMode, "ENABLED"))
   {
      InteropHelper::setClientNATDetectionMode(InteropHelper::ClientNATDetectionEnabled);
   }
   else if(isEqualNoCase(clientNATDetectionMode, "PRIVATE_TO_PUBLIC"))
   {
      InteropHelper::setClientNATDetectionMode(InteropHelper::ClientNATDetectionPrivateToPublicOnly);
   }
   ConnectionManager::MinimumGcHeadroom = mProxyConfig->getConfigUnsignedLong("TCPMinimumGCHeadroom", 0);
   unsigned long tcpConnectionGCAge = mProxyConfig->getConfigUnsignedLong("TCPConnectionGCAge", 0);
   if(tcpConnectionGCAge > 0)
   {
      ConnectionManager::MinimumGcAge = tcpConnectionGCAge * 1000;
      ConnectionManager::EnableAgressiveGc = true;
   }
   unsigned long outboundFlowTimer = mProxyConfig->getConfigUnsignedLong("FlowTimer", 0);
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
   bool assumePath = mProxyConfig->getConfigBool("AssumePath", false);
   bool forceRecordRoute = mProxyConfig->getConfigBool("ForceRecordRouting", false);
   Uri recordRouteUri;
   mProxyConfig->getConfigValue("RecordRouteUri", recordRouteUri);
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
   }

   // Configure misc. stack settings
   mSipStack->setFixBadDialogIdentifiers(false);
   mSipStack->setFixBadCSeqNumbers(false);
   int statsLogInterval = mProxyConfig->getConfigInt("StatisticsLogInterval", 60);
   if(statsLogInterval > 0)
   {
      mSipStack->setStatisticsInterval(statsLogInterval);
      mSipStack->statisticsManagerEnabled() = true;
   }
   else
   {
      mSipStack->statisticsManagerEnabled() = false;
   }
*/
   // Create Congestion Manager, if required
  /* resip_assert(!mCongestionManager);
   if(mProxyConfig->getConfigBool("CongestionManagement", true))
   {
      Data metricData = mProxyConfig->getConfigData("CongestionManagementMetric", "WAIT_TIME", true);
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
         cout << "CongestionManagementMetric specified as an unknown value (" << metricData << "), defaulting to WAIT_TIME.";
      }
      mCongestionManager = new GeneralCongestionManager(
                                          metric,
                                          mProxyConfig->getConfigUnsignedLong("CongestionManagementTolerance", 200));
      mSipStack->setCongestionManager(mCongestionManager);
      }

   // Create base thread to run stack in (note:  stack may use other sub-threads, depending on configuration)
   resip_assert(!mStackThread);
   mStackThread = new EventStackThread(*mSipStack,
                                       *dynamic_cast<EventThreadInterruptor*>(mAsyncProcessHandler),
                                       *mFdPollGrp);
*/
   return true;
}


bool
RegistrarRunner::createDatastore()
{
   // Create Database access objects
   /*resip_assert(!mAbstractDb);
   resip_assert(!mRuntimeAbstractDb);
   int defaultDatabaseIndex = mProxyConfig->getConfigInt("DefaultDatabase", -1);
   if(defaultDatabaseIndex >= 0)
   {
      mAbstractDb = mProxyConfig->getDatabase(defaultDatabaseIndex);
      if(!mAbstractDb)
      {
         CritLog(<<"Failed to get configuration database");
         cleanupObjects();
         return false;
      }
   }
   else     // Try legacy configuration parameter names
   {
#ifdef USE_MYSQL
      Data mySQLServer;
      mProxyConfig->getConfigValue("MySQLServer", mySQLServer);
      if(!mySQLServer.empty())
      {
         WarningLog(<<"Using deprecated parameter MySQLServer, please update to indexed Database definitions.");
         mAbstractDb = new MySqlDb(mySQLServer,
                          mProxyConfig->getConfigData("MySQLUser", Data::Empty),
                          mProxyConfig->getConfigData("MySQLPassword", Data::Empty),
                          mProxyConfig->getConfigData("MySQLDatabaseName", Data::Empty),
                          mProxyConfig->getConfigUnsignedLong("MySQLPort", 0),
                          mProxyConfig->getConfigData("MySQLCustomUserAuthQuery", Data::Empty));
      }
#endif
      if (!mAbstractDb)
      {
         mAbstractDb = new BerkeleyDb(mProxyConfig->getConfigData("DatabasePath", "./", true));
      }
   }
   int runtimeDatabaseIndex = mProxyConfig->getConfigInt("RuntimeDatabase", -1);
   if(runtimeDatabaseIndex >= 0)
   {
      mRuntimeAbstractDb = mProxyConfig->getDatabase(runtimeDatabaseIndex);
      if(!mRuntimeAbstractDb || !mRuntimeAbstractDb->isSane())
      {
         CritLog(<<"Failed to get runtime database");
         cleanupObjects();
         return false;
      }
   }
#ifdef USE_MYSQL
   else     // Try legacy configuration parameter names
   {
      Data runtimeMySQLServer;
      mProxyConfig->getConfigValue("RuntimeMySQLServer", runtimeMySQLServer);
      if(!runtimeMySQLServer.empty())
      {
         WarningLog(<<"Using deprecated parameter RuntimeMySQLServer, please update to indexed Database definitions.");
         mRuntimeAbstractDb = new MySqlDb(runtimeMySQLServer,
                          mProxyConfig->getConfigData("RuntimeMySQLUser", Data::Empty),
                          mProxyConfig->getConfigData("RuntimeMySQLPassword", Data::Empty),
                          mProxyConfig->getConfigData("RuntimeMySQLDatabaseName", Data::Empty),
                          mProxyConfig->getConfigUnsignedLong("RuntimeMySQLPort", 0),
                          mProxyConfig->getConfigData("MySQLCustomUserAuthQuery", Data::Empty));
      }
   }
#endif
   resip_assert(mAbstractDb);
   if(!mAbstractDb->isSane())
   {
      CritLog(<<"Failed to open configuration database");
      cleanupObjects();
      return false;
   }
   if(mRuntimeAbstractDb && !mRuntimeAbstractDb->isSane())
   {
      CritLog(<<"Failed to open runtime configuration database");
      cleanupObjects();
      return false;
   }
   mProxyConfig->createDataStore(mAbstractDb, mRuntimeAbstractDb);

   // Create ImMemory Registration Database
   mRegSyncPort = mProxyConfig->getConfigInt("RegSyncPort", 0);
   // We only need removed records to linger if we have reg sync enabled
   if(!mRestarting)  // If we are restarting then we left the InMemorySyncRegDb and InMemorySyncPubDb intact at restart - don't recreate
   {
      resip_assert(!mRegistrationPersistenceManager);
      mRegistrationPersistenceManager = new InMemorySyncRegDb(mRegSyncPort ? 86400 : 0 );  // !slg! could make linger time a setting
      resip_assert(!mPublicationPersistenceManager);
      mPublicationPersistenceManager = new InMemorySyncPubDb((mRegSyncPort && mProxyConfig->getConfigBool("EnablePublicationRepication", false)) ? true : false);
   }
   resip_assert(mRegistrationPersistenceManager);
   resip_assert(mPublicationPersistenceManager);

   // Copy contacts from the StaticRegStore to the RegistrationPersistanceManager
   populateRegistrations();*/

   return true;
}


void
RegistrarRunner::createRegSync()
{
  /* resip_assert(!mRegSyncClient);
   resip_assert(!mRegSyncServerV4);
   resip_assert(!mRegSyncServerThread);
   if(mRegSyncPort != 0)
   {
      bool enablePublicationReplication = mProxyConfig->getConfigBool("EnablePublicationRepication", false);
      std::list<RegSyncServer*> regSyncServerList;
      if(mUseV4)
      {
         mRegSyncServerV4 = new RegSyncServer(dynamic_cast<InMemorySyncRegDb*>(mRegistrationPersistenceManager),
                                              mRegSyncPort, V4,
                                              enablePublicationReplication ? dynamic_cast<InMemorySyncPubDb*>(mPublicationPersistenceManager) : 0);
         regSyncServerList.push_back(mRegSyncServerV4);
      }
      if(mUseV6)
      {
         mRegSyncServerV6 = new RegSyncServer(dynamic_cast<InMemorySyncRegDb*>(mRegistrationPersistenceManager),
                                              mRegSyncPort, V6,
                                              enablePublicationReplication ? dynamic_cast<InMemorySyncPubDb*>(mPublicationPersistenceManager) : 0);
         regSyncServerList.push_back(mRegSyncServerV6);
      }
      if(!regSyncServerList.empty())
      {
         mRegSyncServerThread = new RegSyncServerThread(regSyncServerList);
      }
      Data regSyncPeerAddress(mProxyConfig->getConfigData("RegSyncPeer", ""));
      if(!regSyncPeerAddress.empty())
      {
         int remoteRegSyncPort = mProxyConfig->getConfigInt("RemoteRegSyncPort", 0);
         if (remoteRegSyncPort == 0)
         {
            remoteRegSyncPort = mRegSyncPort;
         }
         mRegSyncClient = new RegSyncClient(dynamic_cast<InMemorySyncRegDb*>(mRegistrationPersistenceManager),
                                            regSyncPeerAddress, remoteRegSyncPort,
                                            enablePublicationReplication ? dynamic_cast<InMemorySyncPubDb*>(mPublicationPersistenceManager) : 0);
      }
   }*/
}

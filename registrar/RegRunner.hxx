#if !defined(REGRUNNER_HXX)
#define REGRUNNER_HXX
//#include<iostream>
//#include <memory>
#include <map>

using namespace std;

namespace resip
{
  class FdPollGrp;
  class AsyncProcessHandler;
  class SipStack;
  class ThreadIf;
  class CongestionManager;
  class NameAddr;
}

/*namespace repro
{
  class ProxyConfig;
  class AbstractDb;
  class RegSyncClient;
  class RegSyncServer;
  class RegSyncServerThread;
}*/

namespace registrar
{

  class RegConfig;

class RegRunner
{
public:
   RegRunner();
   virtual ~RegRunner();

   virtual bool run(int argc, char** argv);
   virtual void shutdown();
protected:
   virtual void cleanupObjects();//clearn all objects

   virtual bool createSipStack();//create sip stack

   virtual bool addTransports(bool& allTransportsSpecifyRecordRoute); //add transport layer


   bool mRunning; //start or not
   bool mThreadedStack; //ran sip stac or not
   bool mUseV4; //ip4
   bool mUseV6; //ip6
   RegConfig* mRegConfig; //read config file
   resip::FdPollGrp* mFdPollGrp;   //Polling group callback
   resip::AsyncProcessHandler* mAsyncProcessHandler;//will be invoked when Messages are posted to the stack
   resip::SipStack* mSipStack; //sip stack
   resip::ThreadIf* mStackThread; //sip thread


   resip::CongestionManager* mCongestionManager; //congestion manager

   int mArgc;
   char** mArgv;

  /* typedef std::map<unsigned int, resip::NameAddr> TransportRecordRouteMap;
   TransportRecordRouteMap mStartupTransportRecordRoutes;*/
};

}
#endif

#if !defined(REGISTRAR_RUNNER_HXX)
#define REGISTRAR_RUNNER_HXX
#include<iostream>
//#include <repro/ProxyConfig.hxx>
//#include "rutil/Subsystem.hxx"
//#include "resip/stack/StatisticsHandler.hxx"

using namespace std;


namespace resip
{
  /*Subsystem Subsystem::REGISTRAR("REGISTRAR:APP");
  #define RESIPROCATE_SUBSYSTEM Subsystem::REGISTRAR*/

  class FdPollGrp;
  class AsyncProcessHandler;
  class SipStack;
  class ThreadIf;
  class CongestionManager;
}

namespace repro
{
  class ProxyConfig;
  class AbstractDb;
  class RegSyncClient;
  class RegSyncServer;
  class RegSyncServerThread;
}


class RegistrarRunner
{
public:
   RegistrarRunner();
   virtual ~RegistrarRunner();

   virtual bool run(int argc, char** argv);
   virtual void shutdown();
protected:
   bool mRunning;
   bool mUseV4;
   bool mUseV6;
   bool mThreadedStack;
   repro::ProxyConfig* mProxyConfig;
   //resip::ConfigParse* mProxyConfig;
   resip::FdPollGrp *mFdPollGrp;
   resip::AsyncProcessHandler* mAsyncProcessHandler;
   resip::SipStack* mSipStack;
   resip::ThreadIf* mStackThread;

   repro::AbstractDb* mAbstractDb;
   repro::AbstractDb* mRuntimeAbstractDb;
   repro::RegSyncClient* mRegSyncClient;
   repro::RegSyncServer* mRegSyncServerV4;
   repro::RegSyncServerThread* mRegSyncServerThread;

   resip::CongestionManager* mCongestionManager;


   virtual bool createSipStack();
   virtual bool createDatastore();
   virtual void createRegSync();


   int mArgc;
   char** mArgv;
};

#endif

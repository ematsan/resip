#if !defined(REGRUNNER_HXX)
#define REGRUNNER_HXX
#include <map>
#include <rutil/ConfigParse.hxx>

namespace resip
{
  class SipStack;
  class ThreadIf;
  class FdPollGrp;
  class AsyncProcessHandler;
}

using namespace std;

namespace registrar
{
//  #define RESIPROCATE_SUBSYSTEM Subsystem::REGISTRAR

  class RegConfig : public resip::ConfigParse //not abstract class
  {
  public:
     RegConfig(){ cout << "Regconfig constractor"<<endl;}
     virtual ~RegConfig(){cout << "Regconfig destructor"<<endl;}

     virtual void printHelpText(int argc, char **argv){cout << "Regconfig help"<<endl;}
  };


  class RegRunner
  {
  public:
     RegRunner();
     virtual ~RegRunner();

     virtual bool run(int argc, char** argv);
     virtual void shutdown();
  private:
     bool mRunning; //server start(true) or not(false)
     RegConfig* mRegConfig; //read config file
     resip::SipStack* mSipStack; //sip stack
     resip::ThreadIf* mStackThread; //sip thread

     resip::FdPollGrp* mFdPollGrp;   //Polling group callback
     resip::AsyncProcessHandler* mAsyncProcessHandler;//will be invoked when Messages are posted to the stack
   };
}
#endif

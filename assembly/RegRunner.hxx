#if !defined(REGRUNNER_HXX)
#define REGRUNNER_HXX
#include <map>
#include <rutil/ConfigParse.hxx>
#include "rutil/Logger.hxx"
//use to run the sipstack
namespace resip
{
  class SipStack;
  class ThreadIf;
}

using namespace std;

namespace registrar
{
#define RESIPROCATE_SUBSYSTEM resip::Subsystem::TEST

  class RegMySQL;

  class RegConfig : public resip::ConfigParse //not abstract class
  {
  public:
     RegConfig(){ InfoLog(<< "Regconfig constractor");}
     virtual ~RegConfig(){ InfoLog (<< "Regconfig destructor");}

     virtual void printHelpText(int argc, char **argv){InfoLog(<< "Regconfig help");}
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
     RegMySQL* mBase; //mySQL db
   };
}
#endif

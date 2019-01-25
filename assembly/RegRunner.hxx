#pragma once
#include <map>
#include <rutil/ConfigParse.hxx>
#include "rutil/Logger.hxx"

#define RESIPROCATE_SUBSYSTEM resip::Subsystem::TEST

//use to run the sipstack
namespace resip
{
  class SipStack;
  class ThreadIf;
}

namespace registrar
{

  class RegDB;

  class RegConfig : public resip::ConfigParse //not abstract class
  {
  public:
     RegConfig() = default;
     virtual ~RegConfig() = default;
     virtual void printHelpText(int argc, char **argv){}
  };

  class RegRunner
  {
  public:
     RegRunner();
     RegRunner (const RegRunner&) = delete ;
     RegRunner& operator= ( const RegRunner& ) = delete ;
     virtual ~RegRunner();

     virtual bool run(int argc, char** argv);
     virtual void shutdown();
  private:
     bool mRunning; //server start(true) or not(false)
     RegConfig* mRegConfig; //read config file
     resip::SipStack* mSipStack; //sip stack
     resip::ThreadIf* mStackThread; //sip thread
     RegDB* mBase;
   };
}

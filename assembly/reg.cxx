#include <signal.h>
#include <iostream>

#include "RegRunner.hxx"
#include "rutil/Time.hxx"
#include "rutil/Logger.hxx"

using namespace resip;
using namespace std;
using namespace registrar;

static bool finished = false;

static void
signalHandler(int signo)
{
   finished = true;
}

int
main(int argc, char** argv)
{
  //"NONE", "EMERG", "ALERT", "CRIT", "ERR", "WARNING", "NOTICE", "INFO", "DEBUG", "STACK", "CERR", ""
  Log::initialize(Log::Cout, Log::toLevel("debug"), argv[0]);
  //initiate worker for signal
  if ( signal( SIGINT, signalHandler ) == SIG_ERR )
  {
     ErrLog(<<"Couldn't install signal handler for SIGINT");
     exit( -1 );
  }
  if ( signal( SIGTERM, signalHandler ) == SIG_ERR )
  {
     ErrLog(<<"Couldn't install signal handler for SIGTERM");
     exit( -1 );
  }

   InfoLog(<<"Run registrar");
   RegRunner registrar;
   if(!registrar.run(argc, argv))
   {
      ErrLog(<< "Failed to start registrar, exiting...");
      return -1;
   }
   // Main program thread, just waits here for a signal to shutdown
   string s;
   while (!finished)
   {
      //sleepMs(1000);
      InfoLog(<<"Enter commands: exit or restart");
      cin>>s;
      if (s == "exit")
        finished = true;
      if (s == "restart")
      {
        registrar.shutdown();
        if(!registrar.run(argc, argv))
        {
           ErrLog(<< "Failed to start registrar, exiting...");
           finished = true;
        }
      }
    }

  InfoLog(<<"Stop registrar");
  registrar.shutdown();
  return 0;
}

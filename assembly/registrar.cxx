#include <signal.h>
#include <iostream>

#include "RegRunner.hxx"
#include "rutil/Time.hxx"
#include "rutil/Logger.hxx"

using namespace resip;
using namespace std;
using namespace registrar;

static bool finished = false;

//https://ru.cppreference.com/w/cpp/utility/program/signal
static void
signalHandler(int signo)
{
   finished = true;
}

int
main(int argc, char** argv)
{
  //https://ru.cppreference.com/w/cpp/utility/program/signal
  //initiate worker for signal
  Log::initialize(Log::Cout, Log::toLevel("debug"), argv[0]);
  //Log::initialize(Log::Cout, Log::toLevel("err"), argv[0]);
//"NONE", "EMERG", "ALERT", "CRIT", "ERR", "WARNING", "NOTICE", "INFO", "DEBUG", "STACK", "CERR", ""
   //Log::initialize(Log::Cout, Log::toLevel("info"), argv[0]);

  if ( signal( SIGINT, signalHandler ) == SIG_ERR )
  {
     cerr << "Couldn't install signal handler for SIGINT" << endl;
     exit( -1 );
  }

  if ( signal( SIGTERM, signalHandler ) == SIG_ERR )
  {
     cerr << "Couldn't install signal handler for SIGTERM" << endl;
     exit( -1 );
  }

   cout<<"Run registrar"<<endl;
   RegRunner registrar;
   if(!registrar.run(argc, argv))
   {
      cerr << "Failed to start registrar, exiting..." << endl;
      exit(-1);
   }
   // Main program thread, just waits here for a signal to shutdown
   while (!finished)
   {
      sleepMs(1000);
      //cout<<"Waiting"<<endl;
    }

  cout<<"\nStop registrar"<<endl;
  registrar.shutdown();
  return 0;
}

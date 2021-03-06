#include <signal.h>
#include <iostream>

#include "RegRunner.hxx"
#include "rutil/Time.hxx"

using namespace resip;
using namespace std;
using namespace registrar;

static bool finished = false;

//https://ru.cppreference.com/w/cpp/utility/program/signal
static void
signalHandler(int signo)
{
   //cerr << "\nShutting down" << endl;
   finished = true;
}

int main(int argc, char** argv)
{
  //cout<<"Start registrar"<<endl;
  //https://ru.cppreference.com/w/cpp/utility/program/signal
  //initiate worker for signal
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

  //cout<<"\nStop registrar"<<endl;
  registrar.shutdown();
   return 0;
}

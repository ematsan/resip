#include "RegRunner.hxx"
#include "Command.hxx"
#include <iostream>

using namespace std;
using namespace registrar;


CommandThread::CommandThread(int argc, char** argv, bool *fin)
:mArgc(argc),
 mArgv(argv),
 mFinish(fin),
 mRegRanner(0)
{
}

CommandThread::~CommandThread()
{
  if (mRegRanner!=0) mRegRanner->shutdown();
}

void
CommandThread::thread()
{
string s;
mRegRanner = new RegRunner();
if(!mRegRanner->run(mArgc, mArgv))
{
   ErrLog(<< "Failed to start registrar, exiting...");
   shutdown();
}
while(!isShutdown())
{
  //if (kbhit())
  cin>>s;
  if (s == "exit")
  {
    mRegRanner->shutdown();
    *mFinish = true;
    shutdown();
  }
  if (s == "restart")
  {
    mRegRanner->shutdown();
    if(!mRegRanner->run(mArgc, mArgv))
    {
       ErrLog(<< "Failed to start registrar, exiting...");
       shutdown();
    }
  }
}
delete mRegRanner;
mRegRanner = 0;
}

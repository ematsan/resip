#include <iostream>
#include "RegRunner.hxx"

#include <resip/stack/SipStack.hxx>

using namespace resip;
using namespace std;
using namespace registrar;

RegRunner::RegRunner()
   : mRunning(false)
   , mRegConfig(0)
   , mSipStack(0)
{
  cout<<"RegRunner constructor"<<endl;
}

RegRunner::~RegRunner()
{
   cout<<"RegRunner destructor"<<endl;
   if(mRunning) shutdown();
}

bool
RegRunner::run(int argc, char** argv)
{
  cout<<"RegRunner run"<<endl;

  // Parse command line and configuration file
  resip_assert(!mRegConfig);
  Data defaultConfigFilename("reg.config");
  try
  {
     mRegConfig = new RegConfig();
     mRegConfig->parseConfig(argc, argv, defaultConfigFilename);
  }
  catch(BaseException& ex)
  {
     cerr << "Error parsing configuration: " << ex << std::endl;
     return false;
  }

  /* //test mRegConfig
  int port = mRegConfig->getConfigInt("Port", 5080);
  cout<<port<<endl;*/
  // Create SipStack and associated objects
  resip_assert(!mSipStack);
  try
  {
    mSipStack = new SipStack();
    
  }
  catch(BaseException& ex)
  {
     cerr << "Error creating SipStack: " << ex << endl;
     return false;
  }

  mRunning = true;
  return true;
}

void
RegRunner::shutdown()
{
  cout<<"RegRunner shutdown"<<endl;


  delete mSipStack; mSipStack = 0;
  delete mRegConfig; mRegConfig = 0;
  mRunning = false;
}

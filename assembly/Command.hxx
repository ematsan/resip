#if !defined(COMMAND_HXX)
#define COMMAND_HXX

#include "rutil/ThreadIf.hxx"

//work with data
namespace registrar
{
class RegRunner;

class CommandThread : public resip::ThreadIf
{
public:
  CommandThread(int argc, char** argv, bool *fin);
  ~CommandThread();
  void thread();
private:
  RegRunner *mRegRanner;
  int mArgc;
  char** mArgv;
  bool *mFinish;
};
}

#endif

#if !defined(REGTHREAD_HXX)
#define REGTHREAD_HXX

#include "rutil/ThreadIf.hxx"
#include "resip/stack/NameAddr.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::TEST

namespace resip
{
  class SipStack;
}

namespace registrar
{
class RegThread : public resip::ThreadIf
{
    public:

      RegThread(resip::SipStack& stack, resip::NameAddr contact);
      ~RegThread();

      void thread();
   private:
      resip::SipStack& mStack;
      resip::NameAddr mNameAddr;
};
}
#endif

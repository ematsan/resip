#if !defined(REGTHREAD_HXX)
#define REGTHREAD_HXX

#include "rutil/ThreadIf.hxx"
#include "resip/stack/NameAddr.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::TEST

namespace resip
{
  class SipStack;
  class Data;
  class SipMessage;
}

namespace registrar
{
class RegThread : public resip::ThreadIf
{
    public:

      //RegThread(resip::SipStack& stack, resip::NameAddr contact);
      RegThread(resip::SipStack& stack, resip::Data realm);
      ~RegThread();

      void thread();
   private:
      resip::SipStack& mStack;
      resip::Data mNameAddr;


      void send200(resip::SipMessage* sip);
      void send401(resip::SipMessage* sip);
      void send403(resip::SipMessage* sip, resip::Data meth);
};
}
#endif

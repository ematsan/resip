#if !defined(REGTHREAD_HXX)
#define REGTHREAD_HXX

#include "rutil/ThreadIf.hxx"
#include "resip/stack/NameAddr.hxx"

#include "RegDB.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::TEST

namespace resip
{
  class SipStack;
  class Data;
  class SipMessage;
}

namespace registrar
{
class RegMySQL;

class RegThread : public resip::ThreadIf
{
    public:

      //RegThread(resip::SipStack& stack, resip::NameAddr contact);
      RegThread(resip::SipStack& stack, resip::Data realm, RegMySQL* mdatabase);
      ~RegThread();

      void thread();
   private:
      resip::SipStack& mStack;
      resip::Data mNameAddr;
      RegMySQL* mBase;

      void analisysRequest(resip::SipMessage* sip);
      void removeAllContacts(resip::SipMessage* sip);
      bool testAuthorization(resip::SipMessage* sip);
      int testRegistrar(resip::SipMessage* sip);

      void send200(resip::SipMessage* sip, resip::NameAddr add);
      void send400(resip::SipMessage* sip);
      void send401(resip::SipMessage* sip);
      void send403(resip::SipMessage* sip, resip::Data meth);
      void send405(resip::SipMessage* sip, resip::Data meth);

      void loadData();
      RegDB::UserRecordList ulist;
      RegDB::DomainRecordList dlist;
      RegDB::ForwardRecordList flist;
      RegDB::ProtocolRecordList plist;
      RegDB::AuthorizationRecordList alist;
      RegDB::RegistrarRecordList reglist;
      RegDB::RouteRecordList rlist;

};
}
#endif

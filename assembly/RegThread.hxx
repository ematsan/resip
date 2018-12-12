#if !defined(REGTHREAD_HXX)
#define REGTHREAD_HXX

#include "rutil/ThreadIf.hxx"
#include "resip/stack/NameAddr.hxx"

#include "RegDB.hxx"

namespace resip
{
  class SipStack;
  class Data;
  class SipMessage;
}
//work with data
namespace registrar
{
class RegMySQL;

class RegThread : public resip::ThreadIf
{
    public:
      RegThread(resip::SipStack& stack,
                 resip::Data realm,
                 RegMySQL* mdatabase,
                 const std::vector<resip::Data>& configDomains);
      ~RegThread();

      void thread();
   private:
      resip::SipStack& mStack;
      resip::Data mNameAddr;
      RegMySQL* mBase;
      std::vector<resip::Data> mConfigDomains;

      void analisysRequest(resip::SipMessage* sip);
      void removeAllContacts(resip::SipMessage* sip);
      bool testAuthorization(resip::SipMessage* sip);


      int findDomain(resip::Data& host, bool upd);
      int findUser(resip::Data& usr, bool upd);
      int findUserDomain(int usr, int dom, bool upd);
      int findProtocol(resip::Data& protocol, bool upd);
      int findForward(resip::NameAddr& addr, unsigned int reg);
      int findForward(const unsigned int& idp,
                      const unsigned int& idd,
                      const unsigned int& port, bool upd);
      int findRegistrar(resip::SipMessage* sip);
      int findRegistrar(const unsigned int& to,
                      const unsigned int& from,
                      resip::Data& callid, bool upd);

      void send200(resip::SipMessage* sip, resip::NameAddr add);
      void send400(resip::SipMessage* sip);
      void send401(resip::SipMessage* sip);
      void send403(resip::SipMessage* sip, resip::Data mes);
      void send405(resip::SipMessage* sip, resip::Data meth);
      void send500(resip::SipMessage* sip);

      void loadData();
      void clearData();
      void reloadDomain();

      RegDB::UserRecordList ulist;
      RegDB::DomainRecordList dlist;
      RegDB::UserDomainRecordList udlist;
      RegDB::ForwardRecordList flist;
      RegDB::ProtocolRecordList plist;
      RegDB::AuthorizationRecordList alist;
      RegDB::RegistrarRecordList reglist;
      RegDB::RouteRecordList rlist;
};
}
#endif

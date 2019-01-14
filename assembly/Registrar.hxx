#if !defined(REGISTRAR_HXX)
#define REGISTRAR_HXX

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

class Registrar : public resip::ThreadIf
{
    public:
      Registrar(resip::SipStack& stack,
                 resip::Data realm,
                 RegDB* mdatabase,
                 const std::vector<resip::Data>& configDomains);
      ~Registrar();

      void thread();
   private:
      resip::SipStack& mStack;
      resip::Data mNameAddr;
      RegDB* mBase;
      std::vector<resip::Data> mConfigDomains;

      //void analisysRequest(resip::SipMessage* sip);
      void analisysRequest(resip::SipMessage sip);
      void removeAllContacts(resip::SipMessage* sip);
      bool testAuthorization(resip::SipMessage* sip);


      int findDomain(resip::Data& host);
      int addDomain(resip::Data& host);
      int findUser(resip::Data& usr);
      int addUser(resip::Data& usr);
      int findUserDomain(int usr, int dom);
      int addUserDomain(int usr, int dom);
      int findProtocol(resip::Data& protocol);
      int addProtocol(resip::Data& protocol);
      int findForward(resip::NameAddr& addr, unsigned int reg);
      int findForward(const unsigned int& idp,
                      const unsigned int& idd,
                      const unsigned int& port);
      int addForward(const unsigned int& idp,
                      const unsigned int& idd,
                      const unsigned int& port);
      int findRegistrar(resip::SipMessage* sip);
      int findRegistrar(const unsigned int& to,
                      const unsigned int& from,
                      resip::Data& callid);
      int addRegistrar(const unsigned int& to,
                        const unsigned int& from,
                        resip::Data& callid);
      void send200(resip::SipMessage* sip, resip::NameAddr add);
      void send400(resip::SipMessage* sip);
      void send401(resip::SipMessage* sip);
      void send403(resip::SipMessage* sip, resip::Data mes);
      void send405(resip::SipMessage* sip, resip::Data meth);
      void send500(resip::SipMessage* sip);

      void loadData();
      void clearData();
      void reloadDomain();

      RegDB::UserRecordList userList;
      RegDB::DomainRecordList domainList;
      RegDB::UserDomainRecordList userDomeinList;
      RegDB::ForwardRecordList forwardList;
      RegDB::ProtocolRecordList protocolList;
      RegDB::AuthorizationRecordList authList;
      RegDB::RegistrarRecordList regList;
      RegDB::RouteRecordList routeList;
};
}
#endif

#pragma once
#include <vector>
#include "rutil/Data.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::TEST

namespace registrar{

class RegDB{
  public:

    RegDB():mConnected(false){}
    RegDB (const RegDB&) = delete ;
    RegDB& operator= ( const RegDB& ) = delete ;
    virtual ~RegDB() = default;

    virtual int connectDB() const = 0;
    virtual void disconnectDB() const = 0;
    virtual void shutdown() = 0;


    //table of users
    struct UserRecord
    {
          unsigned int mIdUser; //user id
          resip::Data mName; //user name

          UserRecord(unsigned int mIdUser_,
                     resip::Data mName_):
                     mIdUser(mIdUser_),
                     mName(mName_)
                     {}
          UserRecord():
                     mIdUser(0),
                     mName("")
                     {}
    };
    //all domains
    struct DomainRecord
    {
          unsigned int mIdDomain; //domain id
          resip::Data mDomain; //domain name
          unsigned int mIdRealm;//domain realm

          DomainRecord(unsigned int mIdDomain_,
                      resip::Data mDomain_,
                      unsigned int mIdRealm_):
                      mIdDomain(mIdDomain_),
                      mDomain(mDomain_),
                      mIdRealm(mIdRealm_)
                      {}
          DomainRecord():
                      mIdDomain(0),
                      mDomain(""),
                      mIdRealm(0)
                      {}
    };
    //connection users and domains
    struct UserDomainRecord
    {
          unsigned int mIdUserDomain;
          unsigned int mIdDomainFk; //domain id
          unsigned int mIdUserFk;  //user id

          UserDomainRecord(unsigned int mIdUserDomain_,
                      unsigned int mIdDomainFk_,
                      unsigned int mIdUserFk_):
                      mIdUserDomain(mIdUserDomain_),
                      mIdDomainFk(mIdDomainFk_),
                      mIdUserFk(mIdUserFk_)
                      {}
          UserDomainRecord():
                      mIdUserDomain(0),
                      mIdDomainFk(0),
                      mIdUserFk(0)
                      {}
    };
    //table of protocols
    struct ProtocolRecord
    {
          unsigned int mIdProtocol;
          resip::Data mProtocol; //protocol name

          ProtocolRecord(unsigned int mIdProtocol_,
                     resip::Data mProtocol_):
                     mIdProtocol(mIdProtocol_),
                     mProtocol(mProtocol_)
                     {}
          ProtocolRecord():
                     mIdProtocol(0),
                     mProtocol("")
                     {}
    };

    //tables of authorization
    struct AuthorizationRecord
    {
          unsigned int mIdAuth;
          unsigned int mIdUserDomainFk; // who and where have authorization
          resip::Data mPassword; //md5 password

          AuthorizationRecord(unsigned int mIdAuth_,
                      unsigned int mIdUserDomainFk_,
                      resip::Data mPassword_):
                      mIdAuth(mIdAuth_),
                      mIdUserDomainFk(mIdUserDomainFk_),
                      mPassword(mPassword_)
                      {}
          AuthorizationRecord():
                      mIdAuth(0),
                      mIdUserDomainFk(0),
                      mPassword("")
                      {}
    };

    //curent user position
    struct ForwardRecord
    {
          unsigned int mIdForward;
          unsigned int mIdProtocolFk; //protocol
          unsigned int mIdDomainFk; //domain
          unsigned int mPort; //port

          ForwardRecord(unsigned int mIdForward_,
                      unsigned int mIdProtocolFk_,
                      unsigned int mIdDomainFk_,
                      unsigned int mPort_):
                      mIdForward(mIdForward_),
                      mIdProtocolFk(mIdProtocolFk_),
                      mIdDomainFk(mIdDomainFk_),
                      mPort(mPort_)
                      {}
          ForwardRecord():
                      mIdForward(0),
                      mIdProtocolFk(0),
                      mIdDomainFk(0),
                      mPort(0)
                      {}
    };
     //tables of registrators
    struct RegistrarRecord
    {
          unsigned int mIdReg;
          unsigned int mIdUserDomainFk;//whome register
          unsigned int mIdMainFk;//who register
          resip::Data mCallId; //calid

          RegistrarRecord(unsigned int mIdReg_,
                      unsigned int mIdUserDomainFk_,
                      unsigned int mIdMainFk_,
                      resip::Data mCallId_):
                      mIdReg(mIdReg_),
                      mIdUserDomainFk(mIdUserDomainFk_),
                      mIdMainFk(mIdMainFk_),
                      mCallId(mCallId_)
                      {}
          RegistrarRecord():
                      mIdReg(0),
                      mIdUserDomainFk(0),
                      mIdMainFk(0),
                      mCallId(0)
                      {}
    };

    //table of routes
    struct RouteRecord
    {
          unsigned int mIdRoute;
          unsigned int mIdRegFk; //register record (who-whom)
          unsigned int mIdForwardFk; //route record (where)
          resip::Data mTime; //registration time
          unsigned int mExpires; // expires time

          RouteRecord(unsigned int mIdRoute_,
                      unsigned int mIdRegFk_,
                      unsigned int mIdForwardFk_,
                      resip::Data  mTime_,
                      unsigned int mExpires_):
                      mIdRoute(mIdRoute_),
                      mIdRegFk(mIdRegFk_),
                      mIdForwardFk(mIdForwardFk_),
                      mTime(mTime_),
                      mExpires(mExpires_)
                      {}
          RouteRecord():
                      mIdRoute(0),
                      mIdRegFk(0),
                      mIdForwardFk(0),
                      mTime(""),
                      mExpires(0)
                      {}
    };

    typedef unsigned int Key;
    typedef std::vector<UserRecord> UserRecordList;
    typedef std::vector<DomainRecord> DomainRecordList;
    typedef std::vector<UserDomainRecord> UserDomainRecordList;
    typedef std::vector<ProtocolRecord> ProtocolRecordList;
    typedef std::vector<AuthorizationRecord> AuthorizationRecordList;
    typedef std::vector<ForwardRecord> ForwardRecordList;
    typedef std::vector<RegistrarRecord> RegistrarRecordList;
    typedef std::vector<RouteRecord> RouteRecordList;

    // functions for User Records
    virtual void addUser(const UserRecord& rec) const = 0;
    virtual void eraseUser(const Key& key) const = 0;
    virtual int findUserId(const UserRecord& rec) const = 0;
    virtual UserRecord getUser(const Key& key) const = 0;
    virtual UserRecordList getAllUsers() const = 0;
    // functions for Domain Records
    virtual void addDomain(const DomainRecord& rec) const = 0;
    virtual void eraseDomain(const Key& key) const = 0;
    virtual DomainRecord getDomain(const Key& key) const = 0;
    virtual int findDomainId(const DomainRecord& rec) const = 0;
    virtual DomainRecordList getAllDomains() const = 0;
    // functions for User Domain Records
    virtual void addUserDomain(const UserDomainRecord& rec) const = 0;
    virtual void eraseUserDomain(const Key& key) const = 0;
    virtual UserDomainRecord getUserDomain(const Key& key) const = 0;
    virtual int findUserDomainId(const UserDomainRecord& rec) const = 0;
    virtual UserDomainRecordList getAllUserDomains()  const = 0;
    // functions for Protocol Records
    virtual void addProtocol(const ProtocolRecord& rec) const = 0;
    virtual void eraseProtocol(const Key& key) const = 0;
    virtual ProtocolRecord getProtocol(const Key& key) const = 0;
    virtual int findProtocolId(const ProtocolRecord& rec) const = 0;
    virtual ProtocolRecordList getAllProtocols()  const = 0;
    // functions for Authorization Records
    virtual void addAuthorization(const AuthorizationRecord& rec) const = 0;
    virtual void eraseAuthorization(const Key& key) const = 0;
    virtual AuthorizationRecord getAuthorization(const Key& key) const = 0;
    virtual int findAuthorizationId(const AuthorizationRecord& rec) const = 0;
    virtual AuthorizationRecordList getAllAuthorizations() const = 0;
    // functions for Forward Records
    virtual void addForward(const ForwardRecord& rec) const = 0;
    virtual void eraseForward(const Key& key) const = 0;
    virtual ForwardRecord getForward(const Key& key) const = 0;
    virtual int findForwardId(const ForwardRecord& rec) const = 0;
    virtual ForwardRecordList getAllForwards() const = 0;
    // functions for Registrar Records
    virtual void addRegistrar(const RegistrarRecord& rec) const = 0;
    virtual void eraseRegistrar(const Key& key) const = 0;
    virtual RegistrarRecord getRegistrar(const Key& key) const = 0;
    virtual void updateRegistrar(const Key& key, const RegistrarRecord& rec) const = 0;
    virtual int findRegistrarId(const RegistrarRecord& rec) const = 0;
    virtual RegistrarRecordList getAllRegistrars() const = 0;
    // functions for Route Records
    virtual void addRoute(const RouteRecord& rec) const = 0;
    virtual void eraseRoute(const Key& key) const = 0;
    virtual RouteRecord getRoute(const Key& key) const = 0;
    virtual void updateRoute(const Key& key, const RouteRecord& rec) const = 0;
    virtual int findRouteId(const RouteRecord& rec) const = 0;
    virtual RouteRecordList getAllRoutes()  const = 0;

    bool getConnected()
    {
      return mConnected;
    }

protected:
    mutable bool mConnected;

};
}

#if !defined(REGDB_HXX)
#define REGDB_HXX

#include <vector>
#include "rutil/Data.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::TEST

namespace registrar{

class RegDB{
  public:

    RegDB():mConnected(false){}
    virtual ~RegDB() = default;

    //table of users
    struct UserRecord
    {
          unsigned int mIdUser; //user id
          resip::Data mName; //user name
    };
    //all domains
    struct DomainRecord
    {
          unsigned int mIdDomain; //domain id
          resip::Data mDomain; //domain name
          unsigned int mIdRealm;//domain realm
    };
    //connection users and domains
    struct UserDomainRecord
    {
          unsigned int mIdUserDomain;
          unsigned int mIdDomainFk; //domain id
          unsigned int mIdUserFk;  //user id
    };
    //table of protocols
    struct ProtocolRecord
    {
          unsigned int mIdProtocol;
          resip::Data mProtocol; //protocol name
    };

    //tables of authorization
    struct AuthorizationRecord
    {
          unsigned int mIdAuth;
          unsigned int mIdUserDomainFk; // who and where have authorization
          resip::Data mPassword; //md5 password
    };

    //curent user position
    struct ForwardRecord
    {
          unsigned int mIdForward;
          unsigned int mIdProtocolFk; //protocol
          unsigned int mIdDomainFk; //domain
          unsigned int mPort; //port
    };
     //tables of registrators
    struct RegistrarRecord
    {
          unsigned int mIdReg;
          unsigned int mIdUserDomainFk;//whome register
          unsigned int mIdMainFk;//who register
          resip::Data mCallId; //calid
    };

    //table of routes
    struct RouteRecord
    {
          unsigned int mIdRoute;
          unsigned int mIdRegFk; //register record (who-whom)
          unsigned int mIdForwardFk; //route record (where)
          resip::Data mTime; //registration time
          unsigned int mExpires; // expires time
    };

    typedef resip::Data Key;
    typedef std::vector<UserRecord> UserRecordList;
    typedef std::vector<DomainRecord> DomainRecordList;
    typedef std::vector<UserDomainRecord> UserDomainRecordList;
    typedef std::vector<ProtocolRecord> ProtocolRecordList;
    typedef std::vector<AuthorizationRecord> AuthorizationRecordList;
    typedef std::vector<ForwardRecord> ForwardRecordList;
    typedef std::vector<RegistrarRecord> RegistrarRecordList;
    typedef std::vector<RouteRecord> RouteRecordList;

    // functions for User Records
    virtual bool addUser(const UserRecord& rec) const;
    virtual void eraseUser(const Key& key) const;
    virtual UserRecord getUser(const Key& key) const;
    virtual int query(const resip::Data& queryCommand, UserRecord& result) const = 0;
    virtual UserRecordList getAllUsers() const = 0;
    virtual int findUserId(UserRecord& rec) const;

    // functions for Domain Records
    virtual bool addDomain(const DomainRecord& rec) const;
    virtual void eraseDomain(const Key& key) const;
    virtual DomainRecord getDomain(const Key& key) const;
    virtual int query(const resip::Data& queryCommand, DomainRecord& result) const = 0;
    virtual DomainRecordList getAllDomains() const = 0;
    virtual int findDomainId(DomainRecord& rec) const;

    // functions for User Domain Records
    virtual bool addUserDomain(const UserDomainRecord& rec) const;
    virtual void eraseUserDomain(const Key& key) const;
    virtual UserDomainRecord getUserDomain(const Key& key) const;
    virtual int query(const resip::Data& queryCommand, UserDomainRecord& result) const = 0;
    virtual UserDomainRecordList getAllUserDomains()  const = 0;
    virtual int findUserDomainId(UserDomainRecord& rec) const;

    // functions for Protocol Records
    virtual bool addProtocol(const ProtocolRecord& rec) const;
    virtual void eraseProtocol(const Key& key) const;
    virtual ProtocolRecord getProtocol(const Key& key) const;
    virtual int query(const resip::Data& queryCommand, ProtocolRecord& result) const = 0;
    virtual ProtocolRecordList getAllProtocols()  const = 0;
    virtual int findProtocolId(ProtocolRecord& rec) const;

    // functions for Authorization Records
    virtual bool addAuthorization(const AuthorizationRecord& rec) const;
    virtual void eraseAuthorization(const Key& key) const;
    virtual AuthorizationRecord getAuthorization(const Key& key) const;
    virtual int query(const resip::Data& queryCommand, AuthorizationRecord& result) const = 0;
    virtual AuthorizationRecordList getAllAuthorizations() const = 0;
    virtual int findAuthorizationId(AuthorizationRecord& rec) const;

    // functions for Forward Records
    virtual bool addForward(const ForwardRecord& rec) const;
    virtual void eraseForward(const Key& key) const;
    virtual ForwardRecord getForward(const Key& key) const;
    virtual int query(const resip::Data& queryCommand, ForwardRecord& result) const = 0;
    virtual ForwardRecordList getAllForwards() const = 0;
    virtual int findForwardId(ForwardRecord& rec) const;

    // functions for Registrar Records
    virtual bool addRegistrar(const RegistrarRecord& rec) const;
    virtual void eraseRegistrar(const Key& key) const;
    virtual RegistrarRecord getRegistrar(const Key& key) const;
    virtual int query(const resip::Data& queryCommand, RegistrarRecord& result) const = 0;
    virtual RegistrarRecordList getAllRegistrars() const = 0;
    virtual bool updateRegistrar(const Key& key, const RegistrarRecord& rec) const;
    virtual int findRegistrarId(RegistrarRecord& rec) const;

    // functions for Route Records
    virtual bool addRoute(const RouteRecord& rec) const;
    virtual void eraseRoute(const Key& key) const;
    virtual RouteRecord getRoute(const Key& key) const;
    virtual int query(const resip::Data& queryCommand, RouteRecord& result) const = 0;
    virtual RouteRecordList getAllRoutes()  const = 0;
    virtual bool updateRoute(const Key& key, const RouteRecord& rec) const;
    virtual int findRouteId(RouteRecord& rec) const;


    virtual int connectDB() const = 0;
    virtual void disconnectDB() const = 0;
    virtual void shutdown() = 0;

    virtual int query(const resip::Data& queryCommand) const = 0;

    mutable bool mConnected;
};
}
#endif

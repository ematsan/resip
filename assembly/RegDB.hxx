#if !defined(REGDB_HXX)
#define REGDB_HXX

#include <vector>

#include "rutil/Data.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::TEST

namespace registrar{

class RegDB{
  public:
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
    virtual bool addUser(const UserRecord& rec);
    virtual void eraseUser(const Key& key);
    virtual UserRecord getUser(const Key& key) const;
    virtual UserRecordList getAllUsers();
    virtual int findUserId(UserRecord& rec);

    // functions for Domain Records
    virtual bool addDomain(const DomainRecord& rec);
    virtual void eraseDomain(const Key& key);
    virtual DomainRecord getDomain(const Key& key) const;
    virtual DomainRecordList getAllDomains();
    virtual int findDomainId(DomainRecord& rec);

    // functions for User Domain Records
    virtual bool addUserDomain(const UserDomainRecord& rec);
    virtual void eraseUserDomain(const Key& key);
    virtual UserDomainRecord getUserDomain(const Key& key) const;
    virtual UserDomainRecordList getAllUserDomains();
    virtual int findUserDomainId(UserDomainRecord& rec);

    // functions for Protocol Records
    virtual bool addProtocol(const ProtocolRecord& rec);
    virtual void eraseProtocol(const Key& key);
    virtual ProtocolRecord getProtocol(const Key& key) const;
    virtual ProtocolRecordList getAllProtocols();
    virtual int findProtocolId(ProtocolRecord& rec);

    // functions for Authorization Records
    virtual bool addAuthorization(const AuthorizationRecord& rec);
    virtual void eraseAuthorization(const Key& key);
    virtual AuthorizationRecord getAuthorization(const Key& key) const;
    virtual AuthorizationRecordList getAllAuthorizations();
    virtual int findAuthorizationId(AuthorizationRecord& rec);

    // functions for Forward Records
    virtual bool addForward(const ForwardRecord& rec);
    virtual void eraseForward(const Key& key);
    virtual ForwardRecord getForward(const Key& key) const;
    virtual ForwardRecordList getAllForwards();
    virtual int findForwardId(ForwardRecord& rec);

    // functions for Registrar Records
    virtual bool addRegistrar(const RegistrarRecord& rec);
    virtual void eraseRegistrar(const Key& key);
    virtual RegistrarRecord getRegistrar(const Key& key) const;
    virtual RegistrarRecordList getAllRegistrars();
    virtual bool updateRegistrar(const Key& key, const RegistrarRecord& rec);
    virtual int findRegistrarId(RegistrarRecord& rec);

    // functions for Route Records
    virtual bool addRoute(const RouteRecord& rec);
    virtual void eraseRoute(const Key& key);
    virtual RouteRecord getRoute(const Key& key) const;
    virtual RouteRecordList getAllRoutes();
    virtual bool updateRoute(const Key& key, const RouteRecord& rec);
    virtual int findRouteId(RouteRecord& rec);

  protected:

    typedef enum
    {
      UserTable=0,
      DomainTable,
      UserDomainTable,
      ForwardTable,
      ProtocolTable,
      AuthorizationTable,
      RegistrarTable,
      RouteTable,
      MaxTable  // This one MUST be last
    } Table;

    const char tableName[MaxTable][20] = {"tuser", "tdomain", "tuserdomain", "tforward",
      "tprotocol", "tauthorization", "tregistrar", "troute"};

    const char keyName[MaxTable][20] = {"fiduser", "fiddomain", "fidud", "fidforward",
        "fidprotocol", "fidauth", "fidreg", "fidroute"};

    // Db manipulation routines
    // allows deleting records from a table
    virtual void dbEraseRecord(const Table table,
                               const resip::Data& key);

    virtual resip::Data dbKey(const Table table,
                               bool first = false) = 0; // return empty if no more

    virtual int query(const resip::Data& queryCommand, UserRecord& result, const Key& key) const = 0;
    virtual int query(const resip::Data& queryCommand, DomainRecord& result, const Key& key) const = 0;
    virtual int query(const resip::Data& queryCommand, UserDomainRecord& result, const Key& key) const = 0;
    virtual int query(const resip::Data& queryCommand, ProtocolRecord& result, const Key& key) const = 0;
    virtual int query(const resip::Data& queryCommand, AuthorizationRecord& result, const Key& key) const = 0;
    virtual int query(const resip::Data& queryCommand, ForwardRecord& result, const Key& key) const = 0;
    virtual int query(const resip::Data& queryCommand, RegistrarRecord& result, const Key& key) const = 0;
    virtual int query(const resip::Data& queryCommand, RouteRecord& result, const Key& key) const = 0;

    virtual int query(const resip::Data& queryCommand) const = 0;
};
}
#endif

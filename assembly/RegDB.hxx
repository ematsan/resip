#if !defined(REGDB_HXX)
#define REGDB_HXX

#include <mysql/mysql.h>
#include <mysql/errmsg.h>
#include <vector>

#include "rutil/Data.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::TEST

//work with query
namespace registrar{
//create SQL query to DB
class RegDB{
  public:
    struct UserRecord
    {
          unsigned int mIdUser;
          resip::Data mName;
    };
    struct DomainRecord
    {
          unsigned int mIdDomain;
          resip::Data mDomain;
          unsigned int mIdRealm;
    };
    struct UserDomainRecord
    {
          unsigned int mIdUserDomain;
          unsigned int mIdDomainFk;
          unsigned int mIdUserFk;
    };
    struct ProtocolRecord
    {
          unsigned int mIdProtocol;
          resip::Data mProtocol;
    };
    struct AuthorizationRecord
    {
          unsigned int mIdAuth;
          unsigned int mIdUserDomainFk;
          resip::Data mPassword;
    };
    struct ForwardRecord
    {
          unsigned int mIdForward;
          unsigned int mIdProtocolFk;
          unsigned int mIdDomainFk;
          unsigned int mPort;
    };
    struct RegistrarRecord
    {
          unsigned int mIdReg;
          unsigned int mIdUserDomainFk;
          unsigned int mIdMainFk;
          resip::Data mCallId;
    };
    struct RouteRecord
    {
          unsigned int mIdRoute;
          unsigned int mIdRegFk;
          unsigned int mIdForwardFk;
          resip::Data mTime;
          unsigned int mExpires;
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
    virtual int findUser(UserRecord& rec);

    // functions for Domain Records
    virtual bool addDomain(const DomainRecord& rec);
    virtual void eraseDomain(const Key& key);
    virtual DomainRecord getDomain(const Key& key) const;
    virtual DomainRecordList getAllDomains();
    virtual int findDomain(DomainRecord& rec);

    // functions for User Domain Records
    virtual bool addUserDomain(const UserDomainRecord& rec);
    virtual void eraseUserDomain(const Key& key);
    virtual UserDomainRecord getUserDomain(const Key& key) const;
    virtual UserDomainRecordList getAllUserDomains();
    virtual int findUserDomain(UserDomainRecord& rec);

    // functions for Protocol Records
    virtual bool addProtocol(const ProtocolRecord& rec);
    virtual void eraseProtocol(const Key& key);
    virtual ProtocolRecord getProtocol(const Key& key) const;
    virtual ProtocolRecordList getAllProtocols();
    virtual int findProtocol(ProtocolRecord& rec);

    // functions for Authorization Records
    virtual bool addAuthorization(const AuthorizationRecord& rec);
    virtual void eraseAuthorization(const Key& key);
    virtual AuthorizationRecord getAuthorization(const Key& key) const;
    virtual AuthorizationRecordList getAllAuthorizations();
    virtual int findAuthorization(AuthorizationRecord& rec);

    // functions for Forward Records
    virtual bool addForward(const ForwardRecord& rec);
    virtual void eraseForward(const Key& key);
    virtual ForwardRecord getForward(const Key& key) const;
    virtual ForwardRecordList getAllForwards();
    virtual int findForward(ForwardRecord& rec);

    // functions for Registrar Records
    virtual bool addRegistrar(const RegistrarRecord& rec);
    virtual void eraseRegistrar(const Key& key);
    virtual RegistrarRecord getRegistrar(const Key& key) const;
    virtual RegistrarRecordList getAllRegistrars();
    virtual bool updateRegistrar(const Key& key, const RegistrarRecord& rec);
    virtual int findRegistrar(RegistrarRecord& rec);

    // functions for Route Records
    virtual bool addRoute(const RouteRecord& rec);
    virtual void eraseRoute(const Key& key);
    virtual RouteRecord getRoute(const Key& key) const;
    virtual RouteRecordList getAllRoutes();
    virtual bool updateRoute(const Key& key, const RouteRecord& rec);
    virtual int findRoute(RouteRecord& rec);

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

    //mutable MYSQL* mConn;
    // Db manipulation routines
    // allows deleting records from a table
    virtual void dbEraseRecord(const Table table,
                               const resip::Data& key);

    virtual resip::Data dbKey(const Table table,
                               bool first = false) = 0; // return empty if no more

    //virtual int query(const resip::Data& queryCommand, MYSQL_RES** result) const = 0;
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

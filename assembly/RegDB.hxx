#if !defined(REGDB_HXX)
#define REGDB_HXX

#include <mysql/mysql.h>
#include <mysql/errmsg.h>
#include <vector>

#include "rutil/Data.hxx"

using namespace std;

namespace registrar{

class RegDB{
  public:
    struct UserRecord
    {
          resip::Data mUser;
          resip::Data mDomain;
    };

    struct RealmRecord
    {
          resip::Data mDomain;
    };

    struct ForwardRecord
    {
          resip::Data mProtocol;
          resip::Data mAddress;
          resip::Data mIP;
          unsigned int mPort;
    };

    struct ProtocolRecord
    {
          resip::Data mProtocol;
    };

    typedef resip::Data Key;
    typedef vector<UserRecord> UserRecordList;

    // functions for User Records
    virtual bool addUser(const Key& key, const UserRecord& rec);
    virtual void eraseUser(const Key& key);
    /*virtual UserRecord getUser(const Key& key) const;
    virtual resip::Data getUserAuthInfo(const Key& key) const;
    virtual Key firstUserKey();// return empty if no more
    virtual Key nextUserKey(); // return empty if no more

    // functions for Route Records
    virtual bool addRoute(const Key& key, const RouteRecord& rec);
    virtual void eraseRoute(const Key& key);
    virtual RouteRecord getRoute(const Key& key) const;
    virtual RouteRecordList getAllRoutes();
    virtual Key firstRouteKey();// return empty if no more
    virtual Key nextRouteKey(); // return empty if no more

    // functions for Config Records
    virtual bool addConfig(const Key& key, const ConfigRecord& rec);
    virtual void eraseConfig(const Key& key);
    virtual ConfigRecordList getAllConfigs();
    virtual ConfigRecord getConfig(const Key& key) const;
    virtual Key firstConfigKey();// return empty if no more
    virtual Key nextConfigKey(); // return empty if no more*/

  protected:

    typedef enum
    {
      UserTable=0,
      RealmTable,
      ForwardTable,
      ProtocolTable,
      MaxTable  // This one MUST be last
    } Table;

    const char tableName[MaxTable][20] = {"tUser", "tRealm", "tForward", "tProtocol"};

    // Db manipulation routines
    /*virtual bool dbWriteRecord(const Table table,
                           const resip::Data& pKey,
                           const resip::Data& pData) = 0;
    /// return false if not found
    virtual bool dbReadRecord(const Table table,
                              const resip::Data& key,
                              resip::Data& data) const =0;*/
    // allows deleting records from a table that supports secondary keying using a secondary key
    virtual void dbEraseRecord(const Table table,
                               const resip::Data& key);

    virtual int query(const resip::Data& queryCommand, MYSQL_RES** result) const = 0;
    virtual int query(const resip::Data& queryCommand) const = 0;
};

}


#endif

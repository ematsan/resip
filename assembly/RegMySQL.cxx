#include <iostream>
#include <memory>

#include "rutil/Data.hxx"
#include "rutil/Logger.hxx"

#include "RegMySQL.hxx"

using namespace std;
using namespace resip;
using namespace registrar;

RegMySQL::RegMySQL(const resip::Data& server,
          const resip::Data& user,
          const resip::Data& password,
          const resip::Data& databaseName,
          unsigned int port)
    :mDBServer(server)
    ,mDBUser(user)
    ,mDBPassword(password)
    ,mDBName(databaseName)
    ,mDBPort(port)
    ,mConn(nullptr)
    {
      InfoLog(<<"RegMySQL constructor");
      InfoLog( << "Using MySQL DB with server=" << server << ", user=" << user << ", dbName=" << databaseName << ", port=" << port);

      mysql_library_init(0, 0, 0);

      if(!mysql_thread_safe()){
         ErrLog( << "Registrar uses MySQL from multiple threads - you MUST link with a thread safe version of the mySQL client library!");
      }
      else{
         connectDB();
      }

}

RegMySQL::~RegMySQL(){
    InfoLog(<<"RegMySQL destructor");
    try
    {
      if(mConn != nullptr)
          disconnectDB();
    }
    catch(std::exception const& a)
    {
        ErrLog(<<"RegMySQL destructor Throw Exeption: " << a.what());
    }
}

int
RegMySQL::connectDB() const {
  // Disconnect from database first (if required)
  disconnectDB();
  // Now try to connect
  //resip_assert(mConn == nullptr);
  //resip_assert(mConnected == false);
  if (mConn != nullptr)
    {
      ErrLog(<<"mConn != nullptr. Base connected");
      throw MySQLError{};
    }
  if (mConnected)
  {
    ErrLog(<<"mConnected = true, Base connected");
    throw MySQLError{};
  }

  mConn = mysql_init(0);
  if(mConn == nullptr)
  {
     ErrLog( << "MySQL init failed: insufficient memory.");
     return CR_OUT_OF_MEMORY;
  }

  MYSQL* ret = mysql_real_connect(mConn,
                                  mDBServer.c_str(),   // hostname
                                  mDBUser.c_str(),     // user
                                  mDBPassword.c_str(), // password
                                  mDBName.c_str(),     // DB
                                  mDBPort,             // port
                                  0,                   // unix socket file
                                  CLIENT_MULTI_RESULTS); // client flags (enable multiple results, since some custom stored procedures might require this)

  if (ret == nullptr)
  {
     int rc = mysql_errno(mConn);
     ErrLog( << "MySQL connect failed: error=" << rc << ": " << mysql_error(mConn));
     mysql_close(mConn);
     mConn = nullptr;
     mConnected = false;
     return rc;
  }
  else
  {
     InfoLog( << "MySQL connect ok");
     mConnected = true;
     return 0;
  }

}

void
RegMySQL::shutdown()
{
  if(mConn != nullptr)
      disconnectDB();
}

void
RegMySQL::disconnectDB() const
{
  if(mConn != nullptr)
  {
     mysql_close(mConn);
     mConn = nullptr;
     mConnected = false;
  }
}

/*************************************************************************/
/*                        USER                                           */
/*************************************************************************/
bool
RegMySQL::addUser(const UserRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tuser (fname)"
        << " VALUES('"
        << rec.mName << "')";
  }
  return query(command) == 0;
}

bool
RegMySQL::eraseUser(const Key& key) const
{
  Data command;
  {
     DataStream ds(command);
     Data escapedKey;
     ds << "DELETE FROM tuser";
     ds << " WHERE fiduser = '" << key << "'";
  }
  return query(command) == 0;
}

RegDB::UserRecord
RegMySQL::getUser(const Key& key) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fname FROM tuser"
        << " WHERE fiduser='" << key
        << "'";
  }
  UserRecord rec;
  MYSQL_RES* result = nullptr;
  if(query(command, &result) != 0)
     return rec;
  if (result == nullptr)
  {
     ErrLog( << "Base store UserRecord result failed");
     return rec;
  }
  MYSQL_ROW row=mysql_fetch_row(result);
  if(row)
   {
       rec.mIdUser          = key;
       rec.mName            = Data(row[0]);
   }
  mysql_free_result(result);
  return rec;
}

int
RegMySQL::findUserId(const UserRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fiduser FROM tuser"
        << " WHERE fname='" << rec.mName
        << "'";
  }
  MYSQL_RES* result = nullptr;
  if(query(command, &result) != 0)
     return -1;
     //throw MySQLError{};
  if (result == nullptr)
  {
     ErrLog( << "Base store UserRecordId result failed");
     return -2;
     //throw MySQLError{};
  }
  MYSQL_ROW row=mysql_fetch_row(result);
  int res = 0;
  if(row)
     res = Data(row[0]).convertInt();
  mysql_free_result(result);
  if (res == 0)
        ErrLog( << "findUserId result failed");
  return res;
}

RegDB::UserRecordList
RegMySQL::getAllUsers() const
{
  UserRecordList records;
  MYSQL_RES* result = nullptr;
  Data command("SELECT fiduser, fname FROM tuser");

  if(query(command, &result) != 0)
    //throw MySQLError{};
     return records;
  if (result == nullptr)
  {
     ErrLog( << "Base store UserRecordList result failed");
     return records;
    // throw MySQLError{};
  }
  MYSQL_ROW row;
  while(row = mysql_fetch_row(result))
       records.emplace_back(UserRecord(Data(row[0]).convertInt(), Data(row[1])));
  mysql_free_result(result);
  return records;
}

/*************************************************************************/
/*                       DOMAIN                                          */
/*************************************************************************/
bool
RegMySQL::addDomain(const DomainRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tdomain (fdomain, fidrealm)"
        << " VALUES('"
        << rec.mDomain << "', "
        << rec.mIdRealm<<")";
  }
  return query(command) == 0;
}

bool
RegMySQL::eraseDomain(const Key& key) const
{
  Data command;
  {
     DataStream ds(command);
     Data escapedKey;
     ds << "DELETE FROM tdomain";
     ds << " WHERE fiddomain = '" << key << "'";
  }
  return query(command) == 0;
}

RegDB::DomainRecord
RegMySQL::getDomain(const Key& key) const
{
  DomainRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fdomain, fidrealm FROM tdomain"
        << " WHERE fiddomain='" << key
        << "'";
  }
  MYSQL_RES* result = nullptr;
  if(query(command, &result) != 0)
  {
     return rec;
  }
  if (result == nullptr)
  {
     ErrLog( << "DomainRecord result failed");
     return rec;
  }
  MYSQL_ROW row = mysql_fetch_row(result);
  if(row)
   {
       int col = 0;
       rec.mIdDomain  = key;
       rec.mDomain    = Data(row[col++]);
       rec.mIdRealm   = Data(row[col++]).convertInt();
   }
  mysql_free_result(result);
  return rec;
}

int
RegMySQL::findDomainId(const DomainRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fiddomain FROM tdomain"
        << " WHERE fdomain='" << rec.mDomain
        << "'";
  }
  MYSQL_RES* result = nullptr;
  if(query(command, &result) != 0)
  {
     return -2;
  }
  if (result == nullptr)
  {
     ErrLog( << "DomainRecordId result failed");
     return -1;
  }
  MYSQL_ROW row = mysql_fetch_row(result);
  int res = 0;
  if(row)
       res  = Data(row[0]).convertInt();
  mysql_free_result(result);
  if (res == 0)
        ErrLog( << "findDomainId result failed");
  return res;
}

RegDB::DomainRecordList
RegMySQL::getAllDomains() const
{
  DomainRecordList records;
  Data command("SELECT fiddomain, fdomain, fidrealm FROM tdomain");
  MYSQL_RES* result = nullptr;
  if(query(command, &result) != 0)
  {
     return records;
  }
  if (result == nullptr)
  {
     ErrLog( << "Base store DomainRecordList result failed");
     return records;
  }

  MYSQL_ROW row;
  while(row = mysql_fetch_row(result))
   {
       records.emplace_back(DomainRecord(Data(row[0]).convertInt(), Data(row[1]), Data(row[2]).convertInt()));
   }
  mysql_free_result(result);

  return records;
}


/*************************************************************************/
/*                       USER DOMAIN                                     */
/*************************************************************************/
bool
RegMySQL::addUserDomain(const UserDomainRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tuserdomain (fiduserfk, fiddomainfk)"
        << " VALUES("
        << rec.mIdUserFk << ","
        << rec.mIdDomainFk<<")";
  }
  return query(command) == 0;
}

bool
RegMySQL::eraseUserDomain(const Key& key) const
{
  Data command;
  {
     DataStream ds(command);
     Data escapedKey;
     ds << "DELETE FROM tuserdomain";
     ds << " WHERE fidud = '" << key << "'";
  }
  return query(command) == 0;
}

RegDB::UserDomainRecord
RegMySQL::getUserDomain(const Key& key) const
{
  UserDomainRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fiddomainfk, fiduserfk FROM tuserdomain"
        << " WHERE fidud='" << key
        << "'";
  }
  MYSQL_RES* result = nullptr;
    if(query(command, &result) != 0)
    {
       return rec;
    }
    if (result == nullptr)
    {
       ErrLog( << "getUserDomain result failed");
       return rec;
    }

    MYSQL_ROW row=mysql_fetch_row(result);
    if(row)
     {
         int col = 0;
         rec.mIdUserDomain     = key;
         rec.mIdDomainFk       = Data(row[col++]).convertInt();
         rec.mIdUserFk          = Data(row[col++]).convertInt();
     }
    mysql_free_result(result);
    return rec;
}

int
RegMySQL::findUserDomainId(const UserDomainRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidud FROM tuserdomain"
        << " WHERE fiddomainfk = '" << rec.mIdDomainFk
        << "' and fiduserfk = '" << rec.mIdUserFk
        << "'";
  }
  MYSQL_RES* result = nullptr;
    if(query(command, &result) != 0)
    {
       return -2;
    }
    if (result == nullptr)
    {
       ErrLog( << "Base store UserDomainRecord result failed");
       return -1;
    }
    int res = 0;
    MYSQL_ROW row=mysql_fetch_row(result);
    if(row)
     {
         res = Data(row[0]).convertInt();
     }
    mysql_free_result(result);
  if (res == 0)
     ErrLog( << "findUserDomainId result failed");
  return res;
}

RegDB::UserDomainRecordList
RegMySQL::getAllUserDomains() const
{
  UserDomainRecordList records;
  MYSQL_RES* result = nullptr;
  Data command("SELECT fidud, fiddomainfk, fiduserfk FROM tuserdomain");

  if(query(command, &result) != 0)
  {
     return records;
  }
  if (result == nullptr)
  {
     ErrLog( << "Base store UserDomainRecordList result failed");
     return records;
  }

  MYSQL_ROW row;
  while(row = mysql_fetch_row(result))
   {
    records.emplace_back(UserDomainRecord(Data(row[0]).convertInt(), Data(row[1]).convertInt(), Data(row[2]).convertInt()));
   }
  mysql_free_result(result);

  return records;
}



/*************************************************************************/
/*                        PROTOCOL                                       */
/*************************************************************************/
bool
RegMySQL::addProtocol(const ProtocolRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tprotocol (fprotocol)"
        << " VALUES('"
        << rec.mProtocol << "')";
    }
  return query(command) == 0;
}

bool
RegMySQL::eraseProtocol(const Key& key) const
{
  Data command;
  {
     DataStream ds(command);
     Data escapedKey;
     ds << "DELETE FROM tprotocol";
     ds << " WHERE fidprotocol = '" << key << "'";
  }
  return query(command) == 0;
}

RegDB::ProtocolRecord
RegMySQL::getProtocol(const Key& key) const
{
  ProtocolRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fprotocol FROM tprotocol"
        << " WHERE fidprotocol='" << key
        << "'";
  }
    MYSQL_RES* result = nullptr;
    if(query(command, &result) != 0)
    {
       return rec;
    }
    if (result == nullptr)
    {
       ErrLog( << "Base store ProtocolRecord result failed");
       return rec;
    }

    MYSQL_ROW row=mysql_fetch_row(result);
    if(row)
     {
         rec.mIdProtocol          = key;
         rec.mProtocol            = Data(row[0]);
     }
    mysql_free_result(result);
    return rec;
}


int
RegMySQL::findProtocolId(const ProtocolRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidprotocol FROM tprotocol"
        << " WHERE fprotocol = '" << rec.mProtocol
        << "'";
  }
  MYSQL_RES* result = nullptr;
    if(query(command, &result) != 0)
    {
       return -2;
    }
    if (result == nullptr)
    {
       ErrLog( << "Base store ProtocolRecord result failed");
       return -1;
    }

    MYSQL_ROW row=mysql_fetch_row(result);
    int res = 0;
    if(row)
     {
         res   = Data(row[0]).convertInt();
     }
    mysql_free_result(result);

 if (res == 0)
     ErrLog( << "findProtocolId result failed");
  return res;
}

RegDB::ProtocolRecordList
RegMySQL::getAllProtocols() const
{
  ProtocolRecordList records;
  MYSQL_RES* result = nullptr;
  Data command("SELECT fidprotocol, fprotocol FROM tprotocol");

  if(query(command, &result) != 0)
  {
     return records;
  }
  if (result == nullptr)
  {
     ErrLog( << "Base store ProtocolRecordList result failed");
     return records;
  }

  MYSQL_ROW row;
  while(row = mysql_fetch_row(result))
   {
       records.emplace_back(ProtocolRecord(Data(row[0]).convertInt(), Data(row[1])));
   }
  mysql_free_result(result);

  return records;
}


/*************************************************************************/
/*                        FORWARD                                        */
/*************************************************************************/
bool
RegMySQL::addForward(const ForwardRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tforward (fidprotocolfk, fiddomainfk, fport)"
        << " VALUES("
        << rec.mIdProtocolFk << ", "
        << rec.mIdDomainFk << ", "
        << rec.mPort << ")";
  }
  return query(command) == 0;
}

bool
RegMySQL::eraseForward(const Key& key) const
{
  Data command;
  {
     DataStream ds(command);
     Data escapedKey;
     ds << "DELETE FROM tforward";
     ds << " WHERE fidforward = '" << key << "'";
  }
  return query(command) == 0;
}

RegDB::ForwardRecord
RegMySQL::getForward(const Key& key) const
{
  ForwardRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidprotocolfk, fiddomainfk, fport FROM tforward"
        << " WHERE fidforward='" << key
        << "'";
  }
  MYSQL_RES* result = nullptr;
  if(query(command, &result) != 0)
  {
     return rec;
  }
  if (result == nullptr)
  {
     ErrLog( << "Base store ForwardRecord result failed");
     return rec;
  }

  MYSQL_ROW row=mysql_fetch_row(result);
  if(row)
   {
       int col = 0;
       rec.mIdForward        = key;
       rec.mIdProtocolFk     = Data(row[col++]).convertInt();
       rec.mIdDomainFk       = Data(row[col++]).convertInt();
       rec.mPort             = Data(row[col++]).convertInt();
   }
  mysql_free_result(result);
  return rec;
}

int
RegMySQL::findForwardId(const ForwardRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidforward FROM tforward"
        << " WHERE fiddomainfk = '" << rec.mIdDomainFk
        << "' and fidprotocol = '" << rec.mIdProtocolFk
        << "' and fport = '" << rec.mPort
        << "'";
  }
  MYSQL_RES* result = nullptr;
  if(query(command, &result) != 0)
  {
     return -2;
  }
  if (result == nullptr)
  {
     ErrLog( << "Base store ForwardRecord result failed");
     return -1;
  }
  int res = 0;
  MYSQL_ROW row=mysql_fetch_row(result);
  if(row)
   {
       res   = Data(row[0]).convertInt();
   }
  mysql_free_result(result);
  if (res == 0)
     ErrLog( << "findForwardId result failed");
  return res;
}

RegDB::ForwardRecordList
RegMySQL::getAllForwards() const
{
  ForwardRecordList records;
  MYSQL_RES* result = nullptr;
  Data command("SELECT fidforward, fidprotocolfk, fiddomainfk, fport FROM tforward");

  if(query(command, &result) != 0)
  {
     return records;
  }
  if (result == nullptr)
  {
     ErrLog( << "Base store ForwardRecordList result failed");
     return records;
  }

  MYSQL_ROW row;
  while(row = mysql_fetch_row(result))
   {
    records.emplace_back(ForwardRecord(Data(row[0]).convertInt(), Data(row[1]).convertInt(),
                         Data(row[2]).convertInt(), Data(row[3]).convertInt()));
   }
  mysql_free_result(result);


  return records;
}

/*************************************************************************/
/*                        AUTHORIZATION                                  */
/*************************************************************************/
bool
RegMySQL::addAuthorization(const AuthorizationRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tauthorization (fidudfk, fpassword)"
        << " VALUES("
        << rec.mIdUserDomainFk << ", '"
        << rec.mPassword << "')";
  }
  return query(command) == 0;
}

bool
RegMySQL::eraseAuthorization(const Key& key) const
{
  Data command;
  {
     DataStream ds(command);
     Data escapedKey;
     ds << "DELETE FROM tauthorization";
     ds << " WHERE fidauth = '" << key << "'";
  }
  return query(command) == 0;
}

RegDB::AuthorizationRecord
RegMySQL::getAuthorization(const Key& key) const
{
  AuthorizationRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidudfk, fpassword FROM tauthorization"
        << " WHERE fidauth='" << key
        << "'";
  }
  MYSQL_RES* result = nullptr;
  if(query(command, &result) != 0)
  {
     return rec;
  }
  if (result == nullptr)
  {
     ErrLog( << "Base store AuthorizationRecord result failed");
     return rec;
  }

  MYSQL_ROW row=mysql_fetch_row(result);
  if(row)
   {
       int col = 0;
       rec.mIdAuth            = key;
       rec.mIdUserDomainFk    = Data(row[col++]).convertInt();
       rec.mPassword          = Data(row[col++]);
   }
  mysql_free_result(result);
}

int
RegMySQL::findAuthorizationId(const AuthorizationRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidauth FROM tauthorization"
        << " WHERE fidudfk = '" << rec.mIdUserDomainFk
        << "' and fpassword = '" << rec.mPassword
        << "'";
  }

  MYSQL_RES* result = nullptr;
  if(query(command, &result) != 0)
  {
     return -2;
  }
  if (result == nullptr)
  {
     ErrLog( << "Base store AuthorizationRecord result failed");
     return -1;
  }

  MYSQL_ROW row=mysql_fetch_row(result);
  int res = 0;
  if(row)
   {
       res   = Data(row[0]).convertInt();
    }
  mysql_free_result(result);
  if (res == 0)
     ErrLog( << "findAuthorizationId result failed");
  return res;
}


RegDB::AuthorizationRecordList
RegMySQL::getAllAuthorizations() const
{
  AuthorizationRecordList records;
  MYSQL_RES* result = nullptr;
  Data command("SELECT fidauth, fidudfk, fpassword FROM tauthorization");

  if(query(command, &result) != 0)
  {
     return records;
  }
  if (result == nullptr)
  {
     ErrLog( << "Base store AuthorizationRecordList result failed");
     return records;
  }

  MYSQL_ROW row;
  while(row = mysql_fetch_row(result))
   {
       records.emplace_back(AuthorizationRecord(Data(row[0]).convertInt(), Data(row[1]).convertInt(),
                            Data(row[2])));
   }
  mysql_free_result(result);

  return records;
}

/*************************************************************************/
/*                        REGISTRAR                                      */
/*************************************************************************/
bool
RegMySQL::addRegistrar(const RegistrarRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tregistrar (fidudfk, fcallid, fidmainfk)"
        << " VALUES("
        << rec.mIdUserDomainFk << ", '"
        << rec.mCallId<<"', "
        << rec.mIdMainFk << ")";
  }
  return query(command) == 0;
}

bool
RegMySQL::eraseRegistrar(const Key& key) const
{
  Data command;
  {
     DataStream ds(command);
     Data escapedKey;
     ds << "DELETE FROM tregistrar";
     ds << " WHERE fidreg = '" << key << "'";
  }
  return query(command) == 0;
}

RegDB::RegistrarRecord
RegMySQL::getRegistrar(const Key& key) const
{
  RegistrarRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidudfk, fcallid, fidmainfk FROM tregistrar"
        << " WHERE fidreg='" << key
        << "'";
  }
  MYSQL_RES* result = nullptr;
    if(query(command, &result) != 0)
    {
       return rec;
    }
    if (result == nullptr)
    {
       ErrLog( << "Base store RegistrarRecord result failed");
       return rec;
    }

    MYSQL_ROW row=mysql_fetch_row(result);
    if(row)
     {
         int col = 0;
         rec.mIdReg             = key;
         rec.mIdUserDomainFk    = Data(row[col++]).convertInt();
         rec.mCallId            = Data(row[col++]);
         rec.mIdMainFk          = Data(row[col++]).convertInt();
     }
    mysql_free_result(result);
  return rec;
}

bool
RegMySQL::updateRegistrar(const Key& key, const RegistrarRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "UPDATE tregistrar SET"
        << " fcallid = '" << rec.mCallId
        << "' WHERE fidreg = " << key;
  }
  return query(command) == 0;
}

int
RegMySQL::findRegistrarId(const RegistrarRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidreg FROM tregistrar"
        << " WHERE fidudfk = '" << rec.mIdUserDomainFk
        << "' and fidmainfk = '" << rec.mIdMainFk
        << "' and fcallid = '" << rec.mCallId
        << "'";
  }
  MYSQL_RES* result = nullptr;
    if(query(command, &result) != 0)
    {
       return -2;
    }
    if (result == nullptr)
    {
       ErrLog( << "Base store RegistrarRecord result failed");
       return -1;
    }

    MYSQL_ROW row=mysql_fetch_row(result);
    int res = 0;
    if(row)
     {
         res = Data(row[0]).convertInt();
     }
    mysql_free_result(result);

  if (res == 0)
     ErrLog( << "findRegistrarId result failed");
  return res;
  }

RegDB::RegistrarRecordList
RegMySQL::getAllRegistrars() const
{
  RegistrarRecordList records;
  MYSQL_RES* result = nullptr;
  Data command("SELECT fidreg, fidudfk, fidmainfk, fcallid FROM tregistrar");

  if(query(command, &result) != 0)
  {
     return records;
  }
  if (result == nullptr)
  {
     ErrLog( << "Base store RegistrarRecordList result failed");
     return records;
  }

  MYSQL_ROW row;
  while(row = mysql_fetch_row(result))
   {
       records.emplace_back(RegistrarRecord(Data(row[0]).convertInt(), Data(row[1]).convertInt(),
                            Data(row[2]).convertInt(), Data(row[3])));
   }
  mysql_free_result(result);

  return records;
}



/*************************************************************************/
/*                        ROUTE                                          */
/*************************************************************************/
bool
RegMySQL::addRoute(const RouteRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO troute (fidregfk, fidforwardfk, ftime, fexpires)"
        << " VALUES("
        << rec.mIdRegFk << ", "
        << rec.mIdForwardFk << ", '"
        << rec.mTime << "', "
        << rec.mExpires << ")";
  }
  return query(command) == 0;
}

bool
RegMySQL::eraseRoute(const Key& key) const
{
  Data command;
  {
     DataStream ds(command);
     Data escapedKey;
     ds << "DELETE FROM troute";
     ds << " WHERE fidroute = '" << key << "'";
  }
  return query(command) == 0;
}

RegDB::RouteRecord
RegMySQL::getRoute(const Key& key) const
{
  RouteRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidregfk, fidforwardfk, ftime, fexpires FROM troute"
        << " WHERE fidroute='" << key
        << "'";
  }

  MYSQL_RES* result = nullptr;
    if(query(command, &result) != 0)
    {
       return rec;
    }
    if (result == nullptr)
    {
       ErrLog( << "Base store RouteRecord result failed");
       return rec;
     }

    MYSQL_ROW row=mysql_fetch_row(result);
    if(row)
     {
         int col = 0;
         rec.mIdRoute            = key;
         rec.mIdRegFk              = Data(row[col++]).convertInt();
         rec.mIdForwardFk          = Data(row[col++]).convertInt();
         rec.mTime               = Data(row[col++]);
         rec.mExpires            = Data(row[col++]).convertInt();
     }
    mysql_free_result(result);
    return rec;
}

bool
RegMySQL::updateRoute(const Key& key, const RouteRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "UPDATE troute SET"
        << " fidregfk = " << rec.mIdRegFk
        << ", fidforwardfk = " << rec.mIdForwardFk
        << ", ftime = '" << rec.mTime
        << "', fexpires = " << rec.mExpires
        << " WHERE fidroute = " << key;
  }
  return query(command) == 0;
}

int
RegMySQL::findRouteId(const RouteRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidroute FROM troute"
        << " WHERE fidregfk = '" << rec.mIdRegFk
        << "' and fidforwardfk = '" << rec.mIdForwardFk
        << "'";
  }
  MYSQL_RES* result = nullptr;
    if(query(command, &result) != 0)
    {
       return -1;
    }
    if (result == nullptr)
    {
       ErrLog( << "Base store RouteRecord result failed");
       return -2;
     }

    MYSQL_ROW row=mysql_fetch_row(result);
    int res = 0;
    if(row)
     {
         res  = Data(row[0]).convertInt();
     }
    mysql_free_result(result);

  if (res == 0)
     ErrLog( << "findRouteId result failed");
  return res;
}

RegDB::RouteRecordList
RegMySQL::getAllRoutes() const
{
  RouteRecordList records;
  MYSQL_RES* result = nullptr;
  Data command("SELECT fidroute, fidregfk, fidforwardfk, ftime, fexpires FROM troute");

  if(query(command, &result) != 0)
  {
     return records;
  }
  if (result == nullptr)
  {
     ErrLog( << "Base store RouteRecordList result failed");
     return records;
  }

  MYSQL_ROW row;
  while(row = mysql_fetch_row(result))
   {
       records.emplace_back(RouteRecord(Data(row[0]).convertInt(), Data(row[1]).convertInt(),
                            Data(row[2]).convertInt(), Data(row[3]),
                            Data(row[4]).convertInt()));
   }
  mysql_free_result(result);

  return records;
}
/*************************************************************************/
/*                        query                                          */
/*************************************************************************/
int
RegMySQL::query(const Data& queryCommand, MYSQL_RES** result) const
{
   int rc = 0;
   DebugLog( << "RegMySQL::query: executing query: " << queryCommand);
   std::lock_guard<std::mutex> lk(mMutex);
   if(mConn == nullptr || !mConnected)    rc = connectDB();
   if(rc == 0)
   {
      if (mConn == nullptr)
        {
          ErrLog(<<"mConn == nullptr. Base not connected");
          throw MySQLError{};
        }
      if (!mConnected)
      {
        ErrLog(<<"mConnected == false. Base not connected");
        throw MySQLError{};
      }
      rc = mysql_query(mConn,queryCommand.c_str());
      if(rc != 0)
      {
         rc = mysql_errno(mConn);
         if(rc == CR_SERVER_GONE_ERROR ||
            rc == CR_SERVER_LOST)
         {
            // First failure is a connection error - try to re-connect and then try again
            rc = connectDB();
            if(rc == 0)
            {
               // OK - we reconnected - try query again
               rc = mysql_query(mConn,queryCommand.c_str());
               if( rc != 0)
               {
                  ErrLog( << "MySQL query failed: error=" << mysql_errno(mConn) << ": " << mysql_error(mConn));
                  throw MySQLError{};
               }
            }
         }
         else  {
            ErrLog( << "MySQL query failed: error=" << mysql_errno(mConn) << ": " << mysql_error(mConn));
            throw MySQLError{};
         }
      }
   }
   // Now store result - if pointer to result pointer was supplied and no errors
   if(rc == 0 && result)  {
      *result = mysql_store_result(mConn);
      if(*result == 0)
      {
         rc = mysql_errno(mConn);
         if(rc != 0)
         {
            ErrLog( << "MySQL store result failed: error=" << rc << ": " << mysql_error(mConn));
            throw MySQLError{};
         }
      }
      if (result == nullptr)
          throw MySQLError{};
   }
   if(rc != 0) {
      ErrLog( << " SQL Command was: " << queryCommand) ;
      throw MySQLError{};
   }
   return rc;
}

int
RegMySQL::query(const Data& queryCommand) const
{
  return query(queryCommand, 0);
}

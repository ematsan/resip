#include "RegDB.hxx"
#include "rutil/Data.hxx"
#include "rutil/Logger.hxx"

using namespace registrar;
using namespace resip;

#define RESIPROCATE_SUBSYSTEM Subsystem::TEST
/*
static void
encodeString(oDataStream& s, const Data& data)
{
   short len = (short)data.size();
   s.write( (char*)(&len) , sizeof( len ) );
   s.write( data.data(), len );
}


static void
decodeString(iDataStream& s, Data& data)
{
   data.clear();

   if(s.eof()) return;

   short len;
   s.read((char*)(&len), sizeof(len));
   if(s.eof()) return;

   // [TODO] This is probably OK for now, but we can do better than this.
   if (len > 8192)
   {
      ErrLog( << "Tried to decode a database record that was much larger (>8k) than expected.  Returning an empty Data instead." );
      return;
   }

   s.read(data.getBuf(len), len);
}*/
/*************************************************************************/
/*                        USER                                           */
/*************************************************************************/
bool
RegDB::addUser(const Key& key, const UserRecord& rec)
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tusers (fname, fid_domain)"
        << " VALUES('"
        << rec.mName << "', '"
        << rec.mIdDomain << "')"
        << " ON DUPLICATE KEY UPDATE"
        << " fuser='" << rec.mName
        << "', fid_domain='" << rec.mIdDomain
        << "'";
  }
  return query(command, 0) == 0;
}


void
RegDB::eraseUser(const Key& key)
{
  dbEraseRecord( UserTable, key);
}

RegDB::UserRecord
RegDB::getUser(const Key& key) const
{
  UserRecord user;
  return user;
}

RegDB::UserRecordList
RegDB::getAllUsers()
{
  UserRecordList users;
  return users;
}

// functions for Domain Records
bool
RegDB::addDomain(const Key& key, const DomainRecord& rec)
{
  return true;
}
void
RegDB::eraseDomain(const Key& key)
{

}

RegDB::DomainRecord
RegDB::getDomain(const Key& key) const
{

}

RegDB::DomainRecordList
RegDB::getAllDomains()
{

}


// functions for Protocol Records
bool
RegDB::addProtocol(const Key& key, const ProtocolRecord& rec)
{

}

void
RegDB::eraseProtocol(const Key& key)
{

}

RegDB::ProtocolRecord
RegDB::getProtocol(const Key& key) const
{

}

RegDB::ProtocolRecordList
RegDB::getAllProtocol()
{

}

// functions for Forward Records
bool
RegDB::addForward(const Key& key, const ForwardRecord& rec)
{

}

void
RegDB::eraseForward(const Key& key)
{

}

RegDB::ForwardRecord
RegDB::getForward(const Key& key) const
{

}

RegDB::ForwardRecordList
RegDB::getAllForwards()
{

}

// functions for Authorization Records
bool
RegDB::addAuthorization(const Key& key, const AuthorizationRecord& rec)
{

}

void
RegDB::eraseAuthorization(const Key& key)
{

}

RegDB::AuthorizationRecord
RegDB::getAuthorization(const Key& key) const
{

}

RegDB::AuthorizationRecordList
RegDB::getAllAuthorizations()
{

}

// functions for Registrar Records
bool
RegDB::addRegistrar(const Key& key, const RegistrarRecord& rec)
{

}

void
RegDB::eraseRegistrar(const Key& key)
{

}

RegDB::RegistrarRecord
RegDB::getRegistrar(const Key& key) const
{

}

RegDB::RegistrarRecordList
RegDB::getAllRegistrars()
{

}

// functions for Route Records
bool
RegDB::addRoute(const Key& key, const RouteRecord& rec)
{

}

void
RegDB::eraseRoute(const Key& key)
{

}

RegDB::RouteRecord
RegDB::getRoute(const Key& key) const
{

}

RegDB::RouteRecordList
RegDB::getAllRoutes()
{
  
}

/*****************************************************************************/

void
RegDB::dbEraseRecord(const Table table,
                        const Data& key)
{
  Data command;
  {
     DataStream ds(command);
     Data escapedKey;
     ds << "DELETE FROM " << tableName[table];
     ds << " WHERE id='" << key << "'";
  }
  query(command);
}

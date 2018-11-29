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
     ds << "INSERT INTO tusers (fuser, fid_domain)"
        << " VALUES('"
        << rec.mUser << "', '"
        << rec.mDomain << "')"
        << " ON DUPLICATE KEY UPDATE"
        << " fuser='" << rec.mUser
        << "', fid_domain='" << rec.mDomain
        << "'";
  }
  return query(command, 0) == 0;
}


void
RegDB::eraseUser(const Key& key)
{
  dbEraseRecord( UserTable, key);
}


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

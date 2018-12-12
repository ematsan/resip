#include <iostream>
#include <ctime>
#include "RegThread.hxx"
#include "RegMySQL.hxx"
#include "RegDB.hxx"

//!!!!!!!!!!!!!!!!!!!!!! - read example using messages
#include "resip/stack/Helper.hxx"
//!!!!!!!!!!!!!!!!!!!!!! - read example using header
#include "resip/stack/SipMessage.hxx"
//!!!!!!!!!!!! resip/stack/NameAddr.hxx - read example using parametrs address
#include "resip/stack/Uri.hxx"
//!!!!!!!!!!!!read example using parametrs auth
#include "resip/stack/Auth.hxx"
#include "resip/stack/SipStack.hxx"
#include "rutil/Logger.hxx"

using namespace resip;
using namespace std;
using namespace registrar;

RegThread::RegThread(SipStack& stack, Data realm, RegMySQL* mdatabase, const vector<Data>& configDomains)
   : mStack(stack)
   , mNameAddr(realm)
   , mBase(mdatabase)
   , mConfigDomains(configDomains)
{
  cout<<"RegThread constructor"<<endl;
  //select all data
  loadData();
  reloadDomain();

}

RegThread::~RegThread()
{
   cout<<"RegThread destructor"<<endl;
   clearData();
}

void
RegThread::reloadDomain()
{
  //if (std::find(configDomains.begin(), configDomains.end(), realm) != configDomains.end())
  bool reload = false;
  /*for (RegDB::DomainRecord rec : dlist)
  {
     bool kept = false;
     for (Data domain: mConfigDomains)
     {
        if (rec.mDomain == domain)
        {
          kept = true;
          break;
        }
     }
     if (!kept)
     {
         mBase->eraseDomain(Data(rec.mIdDomain));
         reload = true;
     }
  }*/

  for (Data domain: mConfigDomains)
  {
    bool kept = false;
    for (RegDB::DomainRecord rec : dlist)
    {
       if (rec.mDomain == domain)
       {
         kept = true;
         break;
       }
    }
    if (!kept)
    {
        RegDB::DomainRecord n;
        n.mDomain = domain;
        mBase->addDomain(n);
        reload = true;
    }
  }

  //if change list of domains - reload all domains
  if (!reload)
  {
    dlist.clear();
    dlist = mBase->getAllDomains();
    if (dlist.empty()) { ErrLog(<< "No element in table tDomain"); }
  }

  RegDB::DomainRecordList tlist = dlist;
  dlist.clear();
  for (Data domain: mConfigDomains)
  {
    for (RegDB::DomainRecord rec : tlist)
    {
       if (rec.mDomain == domain)
       {
         //kept = true;
         dlist.push_back(rec);
         break;
       }
    }
  }


}

void
RegThread::clearData()
{
  ulist.clear();
  dlist.clear();
  udlist.clear();
  flist.clear();
  plist.clear();
  alist.clear();
  reglist.clear();
  rlist.clear();
}

void
RegThread::loadData()
{
  clearData();
  ulist = mBase->getAllUsers();
  if (ulist.empty()){ ErrLog(<< "No element in table tUser"); }
  dlist = mBase->getAllDomains();
  if (dlist.empty()) { ErrLog(<< "No element in table tDomain"); }
  udlist = mBase->getAllUserDomains();
  if (udlist.empty()) { ErrLog(<< "No element in table tUserDomain"); }
  flist = mBase->getAllForwards();
  if (flist.empty()){ ErrLog(<< "No element in table tForward"); }
  plist = mBase->getAllProtocols();
  if (plist.empty())  { ErrLog(<< "No element in table tProtocol"); }
  alist = mBase->getAllAuthorizations();
  if (alist.empty()) { ErrLog(<< "No element in table tAuthorization");}
  reglist = mBase->getAllRegistrars();
  if (reglist.empty()) { ErrLog(<< "No element in table tRegistrar"); }
  rlist = mBase->getAllRoutes();
  if (rlist.empty()) { ErrLog(<< "No element in table tRoute"); }
}

void
RegThread::thread()
{
  InfoLog(<<"This is the Server");
  while (!isShutdown())
  {
     FdSet fdset;
     mStack.buildFdSet(fdset);
     int err = fdset.selectMilliSeconds(0);
     assert (err != -1);
     mStack.process(fdset);
     SipMessage* received = mStack.receive();
     if (received)
     {
        //auto_ptr<SipMessage> forDel(received);
        MethodTypes meth = received->header(h_RequestLine).getMethod();
        InfoLog(<< "Server received: " << getMethodName(meth));
        if ( meth == REGISTER )
        {
          //NameAddrs& contacts = received->header(h_Contacts);
          //ExpiresCategory& expires = received->header(h_Expires);
          if (received->exists(h_Authorizations))
           {
              analisysRequest(received);
          }
          else
          {
              send401(received);
          }
       }
       else
       {
         send405(received, getMethodName(meth));
       }
     }
     delete received;
  }
}

void
RegThread::analisysRequest(resip::SipMessage* sip)
{
  Auths& auth =  sip->header(h_Authorizations);
  //test Authorization
  if (!testAuthorization(sip))
  {
     send403(sip, "User not register");
     //ErrLog(<< "User not register");
     return;
   }
  //test Registrar
  unsigned int idreg = findRegistrar(sip);
  if (0 == idreg)
  {
     send403(sip, "User not have access to add record");
     //ErrLog(<< "No access to add record");
     return;
   }

  if(sip->exists(h_Contacts))
        {
           NameAddr& to = sip->header(h_To);
           NameAddr& from = sip->header(h_From);
           CallId& callid = sip->header(h_CallId);

           Data fuser = from.uri().user();
           Data fhost = from.uri().host();
           Data tuser = to.uri().user();
           Data thost = to.uri().host();
           //registration time
           unsigned int expires = 0;
           if (sip->exists(h_Expires))
           {
             expires = sip->header(h_Expires).value();
           }
           else
           {
             expires = 3600;
           }
           //parse all contacts
           ParserContainer<NameAddr>& contacts = sip->header(h_Contacts);

           for (ParserContainer<NameAddr>::iterator i = contacts.begin(); i != contacts.end(); i++)
                   {
                     //test format
                     if (!i->isWellFormed())
                     {
                       send400(sip);
                       ErrLog(<< "Not well formed");
                       return;
                     }
                     // Check for "Contact: *" style deregistration
                      if (i->isAllContacts())
                      {
                        if (contacts.size() > 1 || expires != 0)
                          {
                              send400(sip);
                              ErrLog(<< "Error us Contact:*");
                              return;
                           }
                        //remove users contacts
                        removeAllContacts(sip);
                        return;
                      }

                      NameAddr addr = *i;

                      if (addr.exists(p_expires))
                         expires = addr.param(p_expires);

                      unsigned int idf = findForward(addr, expires);
                      if (0 == idf)
                      {
                         send500(sip);
                         ErrLog(<< "Not find Forward");
                         return;
                       }
                      //need to delete record
                      bool upd = false;
                      if (0 == expires)
                      {
                        for(RegDB::RouteRecord rec : rlist)
                        {
                          if ((rec.mIdRegFk == idreg)  && (rec.mIdForwardFk == idf))
                          {
                             mBase->eraseRoute(Data(rec.mIdRoute));
                             upd = true;
                           }
                        }
                      }
                      else //need add or update
                      {
                        for(RegDB::RouteRecord rec : rlist)
                        {
                          //if find - update
                          if ((rec.mIdRegFk == idreg)  && (rec.mIdForwardFk == idf))
                          {
                             rec.mExpires = expires;
                             time_t now = time(0);
                             //https://www.tutorialspoint.com/cplusplus/cpp_date_time.htm
                             tm *ltm = localtime(&now);
                             // print various components of tm structure.
                             //http://mycpp.ru/cpp/scpp/cppd_datetime.htm
                             /*cout << "Year:" << 1900 + ltm->tm_year<<endl;
                             cout << "Month: "<< 1 + ltm->tm_mon<< endl;
                             cout << "Day: "<<  ltm->tm_mday << endl;
                             cout << "Time: "<< ltm->tm_hour << ":";
                             cout << ltm->tm_min << ":";
                             cout << ltm->tm_sec << endl;*/
                             rec.mTime = Data(1900 + ltm->tm_year) + "-" +
                                         Data(1 + ltm->tm_mon) + "-" +
                                         Data(ltm->tm_mday) + " "+
                                         Data(ltm->tm_hour) + ":"+
                                         Data(ltm->tm_min) + ":" +
                                         Data(ltm->tm_sec);
                             mBase->updateRoute(Data(rec.mIdRoute), rec);
                             upd = true;
                           }
                        }
                        //not find - add
                        if (!upd)
                        {
                            RegDB::RouteRecord rec;
                            rec.mIdRegFk = idreg;
                            rec.mIdForwardFk = idf;
                            rec.mExpires = expires;
                            time_t now = time(0);
                            tm *ltm = localtime(&now);
                            rec.mTime = Data(1900 + ltm->tm_year) + "-" +
                                        Data(1 + ltm->tm_mon) + "-"+
                                        Data(ltm->tm_mday) + " "+
                                        Data(ltm->tm_hour) + ":"+
                                        Data(ltm->tm_min) + ":" +
                                        Data(ltm->tm_sec);
                            mBase->addRoute(rec);
                            upd = true;
                        }
                      }
                      //to.param(p_tag)
                      //cout<<"\n\n\n\n\n"<<from.param(p_tag)<<"\n\n\n\n\n";
                      //if remove then reload data
                      if (upd)
                      {
                        rlist.clear();
                        rlist = mBase->getAllRoutes();
                      }
                      send200(sip, *i);
                   }
        }
        else ;//no contacts - do nothing
}

void
RegThread::removeAllContacts(resip::SipMessage* sip)
{
  //remove user contacts
  //find registrar information
   unsigned int idreg = findRegistrar(sip);
   if (0 != idreg)
    {
        bool del = false;
        for(RegDB::RouteRecord rec : rlist)
        {
          if (rec.mIdRegFk == idreg)
          {
             mBase->eraseRoute(Data(rec.mIdRoute));
             del = true;
           }
        }
        //if remove then reload data
        if (del)
        {
          rlist.clear();
          rlist = mBase->getAllRoutes();
        }
    }
}

bool
RegThread::testAuthorization(resip::SipMessage* sip)
{
  for (RegDB::AuthorizationRecord auth : alist)
  {
     if (Helper::authenticateRequestWithA1(*sip, mNameAddr, auth.mPassword, 0) == Helper::Authenticated)
        return true;
  }
  return false;
}

int
RegThread::findForward(resip::NameAddr& addr, unsigned int reg)
{
  unsigned int expires = reg;
  Data user = addr.uri().user();
  Data host = addr.uri().host();
  unsigned int port = addr.uri().port();
  Data scheme = addr.uri().scheme();

  unsigned int idp = findProtocol(scheme);
  if (0 == idp)
  {
     ErrLog(<< "Not find Protocol");
     return 0;
  }

  unsigned int idd = findDomain(host, true);
  if (0 == idd)
  {
     ErrLog(<< "Not find Domain");
     return 0;
  }

  unsigned int idf = findForward(idp, idd, port);
  if (0 == idf)
  {
     return idf;
  }

  return idf;
}

int
RegThread::findForward(const unsigned int& idp,
                const unsigned int& idd,
                const unsigned int& port = 0)
{
  unsigned int idf = 0;
  for (RegDB::ForwardRecord rec : flist)
  {
     if ((idp == rec.mIdProtocolFk) &&
         (port == rec.mPort) &&
         (idd == rec.mIdDomainFk))
     {
       idf = rec.mIdForward;
       break;
     }
  }
  //add forward
  if (0 == idf)
  {
     RegDB::ForwardRecord rec;
     rec.mIdProtocolFk = idp;
     rec.mIdDomainFk = idd;
     rec.mPort = port;
     mBase->addForward(rec);
     //reload
     flist.clear();
     flist = mBase->getAllForwards();
     idf = flist.back().mIdForward;
  }
  return idf;
}

int
RegThread::findProtocol(resip::Data& protocol)
{
  unsigned int idp = 0;
  for (RegDB::ProtocolRecord rec : plist)
  {
     if (protocol == rec.mProtocol)
     {
       idp = rec.mIdProtocol;
       break;
     }
  }
  //add protocol
  if (0 == idp)
  {
     RegDB::ProtocolRecord rec;
     rec.mProtocol = protocol;
     mBase->addProtocol(rec);
     //reload
     plist.clear();
     plist = mBase->getAllProtocols();
     idp = plist.back().mIdProtocol;
  }
  return idp;
}

int
RegThread::findDomain(resip::Data& host, bool upd)
{
  unsigned int idd = 0;
  for (RegDB::DomainRecord rec : dlist)
  {
     if (host == rec.mDomain)
     {
       idd = rec.mIdDomain;
       break;
     }
  }
  //add domain
  if ((0 == idd) && (upd))
  {
     RegDB::DomainRecord rec;
     rec.mDomain = host;
     mBase->addDomain(rec);
     //reload
     dlist.clear();
     dlist = mBase->getAllDomains();
     idd = findDomain(host, false);
     //dlist.back().mIdDomain;
  }
  return idd;
}

int
RegThread::findUser(resip::Data& usr, bool upd)
{
  unsigned int idu = 0;
  for (RegDB::UserRecord rec : ulist)
  {
     if (usr == rec.mName)
     {
       idu = rec.mIdUser;
       break;
     }
  }
  //add domain
  if ((0 == idu) && (upd))
  {
     RegDB::UserRecord rec;
     rec.mName = usr;
     mBase->addUser(rec);
     //reload
     ulist.clear();
     ulist = mBase->getAllDomains();
     idu = findUser(host, false);
     //dlist.back().mIdDomain;
  }
  return idu;
}

int
RegThread::findUserDomain(int usr, int dom, bool upd)
{
    int idud = 0;
    for (RegDB::UserDomainRecord rec : udlist)
    {
        if ((usr == rec.mIdUserFk) && (rec.mIdDomainFk == dom))
        {
           idud = rec.mIdUD;
           break;
         }
    }
    //add userdomain
    if ((0 == idud) && (upd))
    {
      RegDB::UserDomainRecord rec;
      rec.mIdUserFk = usr;
      rec.mIdDomainFk = dom;
      mBase->addUserDomain(rec);
      //reload
      udlist.clear();
      udlist = mBase->getAllUserDomains();
      idud = findUserDomain(usr,dom,false);
    }
    return idud;
}

int
RegThread::findRegistrar(resip::SipMessage* sip)
{
  NameAddr& to = sip->header(h_To);
  NameAddr& from = sip->header(h_From);

  Data& callid = sip->header(h_CallId).value();

  Data fuser = from.uri().user();
  Data fhost = from.uri().host();
  Data tuser = to.uri().user();
  Data thost = to.uri().host();
  unsigned int fidd = 0, tidd = 0;
  unsigned int fidu = 0, tidu = 0;
  unsigned int fidud = 0, tidud = 0;
  bool equal = false;

  if ((fuser == tuser) && (thost == fhost))
  {
    equal = true;
  }
  //find id domain
  fidd = findDomain(fhost, false);
  if (equal)
     tidd = fidd;
  else
     tidd = findDomain(thost, false);
  /*for (RegDB::DomainRecord rec : dlist)
  {
      if (fhost == rec.mDomain)
         fidd = rec.mIdDomain;
      if (thost == rec.mDomain)
          tidd = rec.mIdDomain;
      if ((0 != tidd) && (0 != fidd))
         break;
  }*/
  //find id user
  fidd = findUser(fuser, equal);
  if (equal)
     tidu = fidu;
  else
     tidu = findUser(tuser, equal);
  /*for (RegDB::UserRecord rec : ulist)
  {
      if (fuser == rec.mName)
          fidu = rec.mIdUser;
      if (tuser == rec.mName)
          tidu = rec.mIdUser;
      if ((0 != tidu) && (0 != fidu))
      {
         break;
       }
  }*/
  fidd = findUserDomain(fidu, fidd, equal);
  if (equal)
     tidu = fidu;
  else
     tidu = findUser(tidu, tidd, equal);
/*  for (RegDB::UserDomainRecord rec : udlist)
  {
      if ((fidu == rec.mIdUserFk) && (rec.mIdDomainFk == fidd))
      {
         fidud = rec.mIdUD;
       }
      if ((tidu == rec.mIdUserFk) && (rec.mIdDomainFk == tidd))
      {
          tidud = rec.mIdUD;
        }
      if ((0 != tidud) && (0 != fidud))
      {
         break;
       }
  }*/

  //if own registration
/*  if (equal)
  {
     //if absend then add user
     if (0 == tidu)
     {
       RegDB::UserRecord rec;
       rec.mName = fuser;
       mBase->addUser(rec);
       //reload
       ulist.clear();
       ulist = mBase->getAllUsers();
       tidu = ulist.back().mIdUser;
       fidu = tidu;
       InfoLog(<< "Add user");
     }
     //if absend then add user-domain
     if (0 == tidud)
     {
       RegDB::UserDomainRecord rec;
       rec.mIdUserFk = tidu;
       rec.mIdDomainFk = tidd;
       mBase->addUserDomain(rec);
       //reload
       udlist.clear();
       udlist = mBase->getAllUserDomains();
       tidud = udlist.back().mIdUD;
       fidud = tidud;
       InfoLog(<< "Add user-domain");
     }
  }*/
  /*for (RegDB::UserRecord rec : ulist) {
      if ((fuser == rec.mName) && (rec.mIdDomain == fidd))
         fidu = rec.mIdUser;
      if ((tuser == rec.mName) && (rec.mIdDomain == tidd))
          tidu = rec.mIdUser;
      if ((0 != tidu) && (0 != fidu))
         break;  }*/

  if ((0 == fidu) ||
      (0 == tidu) ||
      (0 == fidd) ||
      (0 == tidd) ||
      (0 == tidud) ||
      (0 == fidud) )
      {
        return 0;
      }
  //find registrar
  unsigned int idreg = 0;
  for (RegDB::RegistrarRecord rec : reglist)
  {
    if ((fidud == rec.mIdMainFk) &&
        (tidud == rec.mIdUDFk))
        {
           if(callid != rec.mCallId)
           {
             rec.mCallId = callid;
             mBase->updateRegistrar(Data(rec.mIdReg),rec);
           }
           idreg = rec.mIdReg;
           break;
      }
  }
  //add registrar
  if ((idreg == 0) && (equal))
  {
      RegDB::RegistrarRecord rec;
      rec.mIdUDFk = tidud;
      rec.mIdMainFk = fidud;
      rec.mCallId = callid;
      rec.mIdReg = 0;
      mBase->addRegistrar(rec);
      //reload
      reglist.clear();
      reglist = mBase->getAllRegistrars();
      idreg = reglist.back().mIdReg;
      InfoLog(<< "Add registrar");
  }
  if (0 == idreg)
  {
    ErrLog(<< "No registrat record\n\n\n\n\n");
  }
  return idreg;
}


/******************************************************************************/
/*                         MESSAGES                                           */
/******************************************************************************/
void
RegThread::send200(SipMessage* sip, NameAddr add)
{
  auto_ptr<SipMessage> msg200(Helper::makeResponse(*sip, 200, add));
  mStack.send(*msg200);
  InfoLog(<< "Sent 200 to REGISTER");
}

void
RegThread::send400(SipMessage* sip){
  auto_ptr<SipMessage> msg400(Helper::makeResponse(*sip, 400));
  ErrLog (<< "Sent 400(Bad Request) to REGISTER");
  mStack.send(*msg400);
}

void
RegThread::send401(SipMessage* sip){
  auto_ptr<SipMessage> msg401(Helper::makeWWWChallenge(*sip, mNameAddr/*"localhost"*/, true, false));
  ErrLog (<< "Sent 401(Unauthorized) to REGISTER");
  mStack.send(*msg401);
}

void
RegThread::send405(SipMessage* sip, Data meth)
{
  int IMMethodList[] = {(int) REGISTER };
  const int IMMethodListSize = sizeof(IMMethodList) / sizeof(*IMMethodList);
  auto_ptr<SipMessage> msg405(Helper::make405(*sip, IMMethodList, IMMethodListSize));
  mStack.send(*msg405);
  ErrLog(<< "Sent 405(Method Not Allowed) to "<< meth);
}


void
RegThread::send403(SipMessage* sip, Data mes)
{
  //auto_ptr<SipMessage> msg403(Helper::makeResponse(*sip, 403, mes));
  auto_ptr<SipMessage> msg403(Helper::makeResponse(*sip, 403));
  mStack.send(*msg403);
  ErrLog(<< "Sent 403(Forbidded) to REGISTER :" + mes);
}

void
RegThread::send500(SipMessage* sip){
  auto_ptr<SipMessage> msg500(Helper::makeResponse(*sip, 500));
  ErrLog (<< "Sent 500(Server Internal Error) to REGISTER");
  mStack.send(*msg500);
}

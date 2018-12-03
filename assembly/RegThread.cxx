#include <iostream>
#include "RegThread.hxx"
#include "RegMySQL.hxx"
#include "RegDB.hxx"
//#include "RegRunner.hxx"

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

RegThread::RegThread(SipStack& stack, Data realm, RegMySQL* mdatabase)
   : mStack(stack)
   , mNameAddr(realm)
   , mBase(mdatabase)
{
  cout<<"RegThread constructor"<<endl;
  //select all data
  loadData();
}


RegThread::~RegThread()
{
   cout<<"RegThread destructor"<<endl;
}

void
RegThread::loadData()
{
  ulist = mBase->getAllUsers();
  if (ulist.empty()){ ErrLog(<< "No element in table tUser"); }
  dlist = mBase->getAllDomains();
  if (dlist.empty()) { ErrLog(<< "No element in table tDomain"); }
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
        auto_ptr<SipMessage> forDel(received);
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
  }
}

void
RegThread::analisysRequest(resip::SipMessage* sip)
{
  Auths& auth =  sip->header(h_Authorizations);
  //test authorization
  if(sip->exists(h_Contacts))
        {

           NameAddr& to = sip->header(h_To);
           NameAddr& from = sip->header(h_From);
           CallId& callid = sip->header(h_CallId);

           Data fuser = from.uri().user();
           Data fhost = from.uri().host();
           Data tuser = to.uri().user();
           Data thost = to.uri().host();


           //cout<<"\n\n\n\n\n"<<callid<<"\n\n\n\n\n";
           /*for(RegDB::AuthorizationRecord auth: alist)
           {
             cout<<"\n\n\n\n\n"<<auth.mIdDomain<<"\n\n\n\n\n";
           }*/
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
                       return;
                     }
                     // Check for "Contact: *" style deregistration
                      if (i->isAllContacts())
                      {
                        if (contacts.size() > 1 || expires != 0)
                          {
                              send400(sip);
                              return;
                           }
                        //removeAllContacts(to);
                        return;
                      }

                      NameAddr addr = *i;

                      Data user = addr.uri().user();
                      Data host = addr.uri().host();
                      if (addr.exists(p_expires))
                         expires = addr.param(p_expires);
                      //to.param(p_tag)
                      //cout<<"\n\n\n\n\n"<<from.param(p_tag)<<"\n\n\n\n\n";
                      cout<<"\n\n\n\n\n"<<host<<"\n\n\n\n\n";
                      send200(sip, *i);
                   }
        }
        else
        {
             //send400(sip);
        }
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
  //auto_ptr<SipMessage> msg401(Helper::makeResponse(*received, 401, mNameAddr));
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
  ErrLog(<< "Sent 405 to "<< meth);
}


void
RegThread::send403(SipMessage* sip, Data meth)
{
  auto_ptr<SipMessage> msg403(Helper::makeResponse(*sip, 403));
  mStack.send(*msg403);
  ErrLog(<< "Sent 403 to "<< meth);
}

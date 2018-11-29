#include <iostream>
#include "RegThread.hxx"
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

RegThread::RegThread(SipStack& stack, Data realm)
   : mStack(stack),
     mNameAddr(realm)
{
  cout<<"RegThread constructor"<<endl;
}


RegThread::~RegThread()
{
   cout<<"RegThread destructor"<<endl;
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
            Auths& auth =  received->header(h_Authorizations);
            //test authorization


            if(received->exists(h_Contacts))
                  {
                     NameAddr& to = received->header(h_To);
                     NameAddr& from = received->header(h_From);
                     
                     //parse all contacts
                     ParserContainer<NameAddr>& contacts = received->header(h_Contacts);
                     for (ParserContainer<NameAddr>::iterator i = contacts.begin();
                             i != contacts.end(); i++){
                               auto_ptr<SipMessage> msg200(Helper::makeResponse(*received, 200, *i));
                               mStack.send(*msg200);
                             }
                  }
           InfoLog(<< "Sent 200 to REGISTER");
          }
          else
          {
              send401(received);
          }

       }
       else
       {
         send403(received, getMethodName(meth));
       }
     }
  }
}


void
RegThread::send200(SipMessage* sip){

}

void
RegThread::send401(SipMessage* sip){
  //auto_ptr<SipMessage> msg401(Helper::makeResponse(*received, 401, mNameAddr));
  auto_ptr<SipMessage> msg401(Helper::makeWWWChallenge(*sip, mNameAddr/*"localhost"*/, true, false));
  ErrLog (<< "Sent 401(Unauthorized) to REGISTER");
  mStack.send(*msg401);
}

void
RegThread::send403(SipMessage* sip, Data meth)
{
  auto_ptr<SipMessage> msg403(Helper::makeResponse(*sip, 403));
  mStack.send(*msg403);
  InfoLog(<< "Sent 403 to "<< meth);
}

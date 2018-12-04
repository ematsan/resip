//./client debug "sip:fluffy@127.0.0.1:5070"
//./client debug "sip:yffulf@127.0.0.1:5070"
#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <sys/types.h>
#include <iostream>
#include <memory>
//!!!!!!!!!!!!!!!!!!!!!! - read example using messages
#include "resip/stack/Helper.hxx"
//!!!!!!!!!!!!!!!!!!!!!! - read example using header
#include "resip/stack/SipMessage.hxx"
//!!!!!!!!!!!! resip/stack/NameAddr.hxx - read example using parametrs
#include "resip/stack/Uri.hxx"
#include "resip/stack/SipStack.hxx"
#include "resip/stack/DeprecatedDialog.hxx"
#include "rutil/Logger.hxx"
#include "rutil/ThreadIf.hxx"

using namespace resip;
using namespace std;

#define RESIPROCATE_SUBSYSTEM Subsystem::TEST

class Client
{
    public:
      Client(TransportType transport, const NameAddr& from, const NameAddr& contact, const NameAddr& target)
         : mStack(),
           mContact(contact),
           mFrom(from),
           mTarget(target),
           mMessage(Helper::makeRegister( target, mFrom, mContact))
      {
         mStack.addTransport(transport, contact.uri().port());
         //auto_ptr<SipMessage> message(Helper::makeInvite( target, mContact, mContact));
         auto_ptr<SipMessage> message(Helper::makeRegister( target, mFrom, mContact));
         mStack.send(*message);
      }

      void process(unsigned int timeoutMs)
      {
         mStack.process(timeoutMs);

         SipMessage* received = mStack.receive();
         if (received)
         {
            InfoLog (<< "Client received: " << received->brief());

            auto_ptr<SipMessage> forDel(received);
            if ( (received->isResponse()) )
            {
               if ( received->header(h_StatusLine).responseCode() == 200 )
               {
                     exit(0);
               }
               if ( received->header(h_StatusLine).responseCode() == 401 )
               {
                DebugLog(<< "Sending Auth");
                const Data& cnonce = "156156448";
                unsigned int n = 9;
                Helper::addAuthorization(* mMessage, *received, "test", "test", cnonce, n);
                mStack.send(* mMessage);

               }
            }
         }
      }
   private:
      SipStack mStack;
      NameAddr mContact;
      NameAddr mFrom;
      NameAddr mTarget;
      SipMessage* mMessage;
};

int
main(int argc, char* argv[])
{
   if (argc != 3)
   {
      cerr << argv[0] << " LOG_LEVEL TARGET_URI" << endl;
      exit(-1);
   }
   Log::initialize(Log::Cout, Log::toLevel(argv[1]), argv[0]);

   NameAddr target(argv[2]);
   cout<<argv[2]<<endl;

   NameAddr contact;
   //contact.uri().host() = SipStack::getHostname();
   contact.uri().host() = "127.0.0.1";
   contact.uri().port() = 5080;
   contact.uri().user() = "yffulf";

   TransportType protocol;
   protocol = TCP;
   Client c(protocol, contact, contact, target);

   while (true)
   {
      c.process(0);
   }
}

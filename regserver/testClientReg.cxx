//./client debug "sip:fluffy@127.0.0.1:5070"

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

#include "resip/stack/Helper.hxx"
#include "resip/stack/SipMessage.hxx"
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
           mWaitingForBye200(false)
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
                  if (!mWaitingForBye200)
                  {
                     ErrLog(<< "Creating dialog.");
                     DeprecatedDialog dlog(mContact);

                     DebugLog(<< "Creating dialog as UAC.");
                     dlog.createDialogAsUAC(*received);

                     DebugLog(<< "making ack.");
                     auto_ptr<SipMessage> ack(dlog.makeAck(*received) );
                     DebugLog(<< *ack);

                     DebugLog(<< "making bye.");
                     auto_ptr<SipMessage> bye(dlog.makeBye());

                     DebugLog(<< "Sending ack: << " << endl << *ack);
                     mStack.send(*ack);

                     DebugLog(<< "Sending bye: << " << endl << *bye);
                     mStack.send(*bye);
                     mWaitingForBye200 = true;
                  }
                  else
                  {
                     auto_ptr<SipMessage> message(Helper::makeInvite( mTarget, mContact, mContact));
                     mStack.send(*message);
                     mWaitingForBye200 = false;
                  }
               }
               if ( received->header(h_StatusLine).responseCode() == 401 )
               {
                 auto_ptr<SipMessage> message(Helper::makeRegister(mTarget, mFrom, mContact));
                 message->header(h_Authorizations)//.param(p_username) = "test";
                 mStack.send(*message);
               }
            }
         }
      }
   private:
      SipStack mStack;
      NameAddr mContact;
      NameAddr mFrom;
      NameAddr mTarget;
      bool mWaitingForBye200;
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

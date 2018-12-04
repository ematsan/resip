#include <iostream>
#include <fstream>
#include "resip/stack/Helper.hxx"
#include "rutil/MD5Stream.hxx"

using namespace std;
using namespace resip;

int main(int argc, char** argv)
{
  //cout<<"test"<<endl;
  string realm, user, password;
  cout << "Enter user" << endl;
  cin >> user;
  cout << "Enter realm" << endl;
  cin >> realm;
  cout << "Enter password" << endl;
  cin >> password;
  MD5Stream a1;
  a1 << user
   << Symbols::COLON
   << realm
   << Symbols::COLON
   << password;
   //cout<< a1;
   cout << "MD5 - "<<a1.getHex()<<endl;
   ofstream myfile;
   myfile.open ("result");
   myfile << user << endl;
   myfile << realm << endl;
   myfile << password << endl;
   myfile << a1.getHex();
   myfile.close();
}

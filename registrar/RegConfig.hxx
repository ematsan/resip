#if !defined(REGCONFIG_HXX)
#define REGCONFIG_HXX

#include <rutil/ConfigParse.hxx>

namespace registrar
{

class RegConfig : public resip::ConfigParse
{
public:
   RegConfig();
   virtual ~RegConfig();

   virtual void printHelpText(int argc, char **argv);
};

}
#endif

#include <map>
#include <string>
#include <TH1D.h>
#include <TH2D.h>
//#include "../Libraries/Include/BootstrapTH1D.h"

#ifdef __MAKECINT__
#pragma link C++ class string+;
#pragma link C++ class map+;
#pragma link C++ class map<string,TH1D*>+;
#pragma link C++ class map<string,TH2D*>+;
#pragma link C++ class std::map<std::string,std::map<std::string,TH1D*>>+;
//#pragma link C++ class ubana::BootstrapTH1D+;
#endif

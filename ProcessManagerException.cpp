#include "ProcessManagerException.h"

namespace Process
{


using namespace std;

ProcessManagerException::ProcessManagerException(const char *msg)
: runtime_error( msg )
{}

const char* ProcessManagerException::what() const throw()
{
cnvt.str( "" );

cnvt << runtime_error::what() << endl;

return cnvt.str().c_str();
}

ostringstream ProcessManagerException::cnvt;

}

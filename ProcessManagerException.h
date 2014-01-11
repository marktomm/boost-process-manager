#ifndef H_PROCESS_MANAGER_EXCEPTION
#define H_PROCESS_MANAGER_EXCEPTION

#include <iostream>
#include <exception>
#include <stdexcept>
#include <sstream>

namespace Process
{


using namespace std;

class ProcessManagerException: public runtime_error {
public:

    ProcessManagerException(const char* msg);

    virtual const char* what() const throw();

private:

    static ostringstream cnvt;
};

}

#endif

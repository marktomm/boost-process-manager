#ifndef H_PROCESS_MANAGER_BASE
#define H_PROCESS_MANAGER_BASE

#include <boost/filesystem.hpp>
#include <string>

namespace Process
{

class ProcessManagerBase
{
public:

    virtual ~ProcessManagerBase(){}

private:

    virtual void TerminateProcessImpl(std::string pid_alias) = 0;
    virtual void TerminateAllProcesssesImpl() = 0;
    virtual void WaitImpl() = 0;

    virtual void CheckProcesses() = 0;

    virtual void LaunchProcessImpl(boost::filesystem::path p, std::string arguments, std::string pid_alias) = 0;

};

}

#endif

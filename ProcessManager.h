#ifndef H_PROCESS_MANAGER
#define H_PROCESS_MANAGER

#include <boost/process.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>

#include <deque>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace Process
{

std::vector<std::string> StringToVector(std::string s);

class ProcessScheduler
{
public:

    ProcessScheduler();
    ProcessScheduler( boost::filesystem::path p, std::string arguments, std::string pid_alias);
    ProcessScheduler( std::string s , std::string pid_alias);
    ~ProcessScheduler();

    void LaunchProcess(boost::filesystem::path p, std::string arguments, std::string pid_alias);
    void LaunchShell(std::string s , std::string pid_alias);

    void TerminateProcess(std::string pid_alias);
    void TerminateAllProcesses();

    void ViewProcessPids();
    void ViewProcessPid(std::string pid_alias);

    bool IsAnyProcessRunning();

private:

    void CheckProcesses();

    boost::process::self &mSelf;
    std::map<std::string, boost::process::child> *mChildrenObjectsMap;
    std::string mCurTerminatigPidAlias, mCurCheckPidAlias;

    boost::posix_time::milliseconds mProcessCheckInterval;
    bool mIsProcessCheckInProgress;
    boost::mutex mChildrenObjectsMapLock;
    boost::thread mThreadCheckProcesses;
    std::map<std::string, boost::process::child>::iterator mCurCheckPidIt;
};

}

#endif

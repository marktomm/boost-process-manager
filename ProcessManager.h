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

// TODO : See if it is possible to use boost::thread::id to check for not a thread instead of mIsProcessCheckInProgress
// TODO : Create wait() mehtod to wait for all subprocesses to finish
// TODO : Fix CheckProcesses() to catch finished processes. Mb with kill()
// TODO : Test wheter it is explicitly needed to call terminate() in TerminateAllProcesses()

using namespace std;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::process;
using namespace boost::filesystem;


vector<string> StringToVector(string s);

class ProcessScheduler
{
public:



    ProcessScheduler();
    ProcessScheduler( path p, string arguments, string pid_alias);
    ProcessScheduler( string s , string pid_alias);
    ~ProcessScheduler();

    void LaunchProcess(path p, string arguments, string pid_alias);
    void LaunchShell(string s , string pid_alias);

    void TerminateProcess(string pid_alias);
    void TerminateAllProcesses();

    void ViewProcessPids();
    void ViewProcessPid(string pid_alias);

    bool IsAnyProcessRunning();

private:

    void CheckProcesses();

    self &mSelf;
    map<string, child> *mChildrenObjectsMap;
    string mCurTerminatigPidAlias, mCurCheckPidAlias;

    milliseconds mProcessCheckInterval;
    bool mIsProcessCheckInProgress;
    boost::mutex mChildrenObjectsMapLock;
    boost::thread mThreadCheckProcesses;
    map<string, child>::iterator mCurCheckPidIt;
};

}

#endif

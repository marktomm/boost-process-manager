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

#include "ProcessManagerException.h"

namespace Process
{

// TODO : Create wait() mehtod to wait for all subprocesses to finish
// TODO : Opt CheckProcesses(): Should auto it = mCurCheckPidIt; or overkill with code bload?
// TOOD : Opt ProcessManagerException class: Is best way?
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
    ProcessScheduler(path p, string arguments, string pid_alias);
    ProcessScheduler(string s , string pid_alias);
    ~ProcessScheduler();

    void LaunchProcess(path p, string arguments, string pid_alias);
    void LaunchShell(string s , string pid_alias);

    void TerminateProcess(string pid_alias);
    void TerminateAllProcesses();

    void ViewProcessPids();
    void ViewProcessPid(string pid_alias);

    bool IsAnyProcessRunning();
    void Wait();

private:

    void CheckProcesses();

    void _LaunchProcess(path p, string arguments, string pid_alias);
    void _LaunchShell(string s , string pid_alias);


    self &mSelf;
    map<string, child> *mChildrenObjectsMap;
    string mCurTerminatigPidAlias, mCurCheckPidAlias;

    milliseconds mProcessCheckInterval, mWaitInterval;
    mutex mChildrenObjectsMapLock;
    thread mThreadCheckProcesses;
    thread_group mThreadWaitForProcesses;
    map<string, child>::iterator mCurCheckPidIt;
};

}

#endif

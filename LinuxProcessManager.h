#ifndef H_PROCESS_MANAGER
#define H_PROCESS_MANAGER

#include <boost/process.hpp>
#include <boost/thread.hpp>

#include <deque>
#include <vector>
#include <sstream>
#include <iomanip>
#include <iostream>

#include "ProcessManagerBase.h"
#include "ProcessManagerException.h"

namespace Process
{

// TODO : Opt CheckProcesses(): Should auto it = mCurCheckPidIt; or overkill with code bload?
// TOOD : Opt ProcessManagerException class: Is best way?
// TODO : Test wheter it is explicitly needed to call terminate() in TerminateAllProcesses()
// TODO : Implement IPC mechanism
// TODO : LinuxProcessManager: Launh functions shoudld use posix version
// TODO ; use read mutexes where able

using namespace std;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::process;
using namespace boost::filesystem;


vector<string> StringToVector(string s);

class LinuxProcessManager: public ProcessManagerBase
{
public:



    LinuxProcessManager();
    LinuxProcessManager(path p, string arguments, string pid_alias);
    LinuxProcessManager(string s , string pid_alias);
    ~LinuxProcessManager();

    void LaunchProcess(path p, string arguments, string pid_alias);
    void LaunchShell(string s , string pid_alias);

    void TerminateProcess(string pid_alias);
    void TerminateAllProcesses();

    void ViewProcessPids();
    void ViewProcessPid(string pid_alias);

    bool IsAnyProcessRunning();
    void Wait();

private:


    void TerminateProcessImpl(string pid_alias);
    void TerminateAllProcesssesImpl() ;

    void WaitImpl();

    void CheckProcesses();

    void LaunchProcessImpl(path p, string arguments, string pid_alias);
    void LaunchShellImpl(string s , string pid_alias);


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

#include <boost/process.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <deque>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>

#include <sys/types.h>
#include <signal.h>

#include "ProcessManager.h"

namespace Process
{

using namespace std;
using namespace boost;
using namespace boost::process;

vector<string> StringToVector(string s)
{
    stringstream ss(s);
    istream_iterator<string> begin(ss);
    istream_iterator<string> end;
    vector<string> args(begin, end);
    copy(args.begin(), args.end(), ostream_iterator<string>(cout, "\n"));

    return args;
}



ProcessScheduler::ProcessScheduler()
    :
      mSelf(self::get_instance()),
      mChildrenObjectsMap(new map<string, child>),
      mCurTerminatigPidAlias(""),
      mProcessCheckInterval(1000),
      mThreadWaitForProcesses()
{
      mThreadCheckProcesses = thread(&ProcessScheduler::CheckProcesses, this);
}



ProcessScheduler::ProcessScheduler( filesystem::path p, string arguments, string pid_alias)
    :
      mSelf(self::get_instance()),
      mChildrenObjectsMap(new map<string, child>),
      mCurTerminatigPidAlias(""),
      mProcessCheckInterval(1000),
      mThreadWaitForProcesses()
{
    LaunchProcess(p, arguments, pid_alias);
    mThreadCheckProcesses = thread(&ProcessScheduler::CheckProcesses, this);
}



ProcessScheduler::ProcessScheduler( string s , string pid_alias)
    :
      mSelf(self::get_instance()),
      mChildrenObjectsMap(new map<string, child>),
      mCurTerminatigPidAlias(""),
      mProcessCheckInterval(1000)
{
    LaunchShell(s, pid_alias);
    mThreadCheckProcesses = thread(&ProcessScheduler::CheckProcesses, this);
}



ProcessScheduler::~ProcessScheduler()
{
    TerminateAllProcesses();

    delete mChildrenObjectsMap;
}



void ProcessScheduler::LaunchProcess(filesystem::path p, string arguments, string pid_alias)
{
    mThreadWaitForProcesses.create_thread( bind( &ProcessScheduler::_LaunchProcess, this, p, arguments, pid_alias ) );
}



void ProcessScheduler::LaunchShell(string s , string pid_alias)
{
    mThreadWaitForProcesses.create_thread( bind( &ProcessScheduler::_LaunchShell, this, s, pid_alias ) );
}



void ProcessScheduler::TerminateProcess(string pid_alias)
{
    lock_guard<mutex> lock(mChildrenObjectsMapLock);
    mCurTerminatigPidAlias = pid_alias;
    try
    {
        mChildrenObjectsMap->at(pid_alias).terminate();

    }
    catch(system::system_error& e)
    {
        cerr << "TerminateProcess() = " << e.what() << endl;
        mChildrenObjectsMap->erase(mCurTerminatigPidAlias);
    }
}



void ProcessScheduler::TerminateAllProcesses()
{
    mThreadCheckProcesses.interrupt();
    mThreadWaitForProcesses.interrupt_all();
    bool isFinished = false;
    while(!isFinished)
    {
        try
        {
            /* map<string, child*>::iterator could be instead of auto */
            for(auto it = mChildrenObjectsMap->begin(); it != mChildrenObjectsMap->end(); ++it)
            {
                mCurTerminatigPidAlias = it->first;
                it->second.terminate();

                mChildrenObjectsMap->erase(it);
            }
            isFinished=true;
        }
        catch(system::system_error& e)
        {
            cerr << "TerminateAllProcesses() = " << e.what() << endl;
            mChildrenObjectsMap->erase(mCurTerminatigPidAlias);
        }
    }
}



void ProcessScheduler::ViewProcessPids()
{
    try
    {
        lock_guard<mutex> lock(mChildrenObjectsMapLock);
        /* map<string, child>::iterator could be instead of auto */
        if(!mChildrenObjectsMap->empty())
            for(auto it = mChildrenObjectsMap->begin(); it != mChildrenObjectsMap->end(); ++it)
            {
                cout << "Process id: " << it->second.get_id() << " Name: " << it->first << endl;
            }
        else
            cerr << "Process list empty" << endl;
    }
    catch(system::system_error& e)
    {
        cerr << "ViewProcessPids() = " << e.what() << endl;
    }
}



void ProcessScheduler::ViewProcessPid(string pid_alias)
{
    lock_guard<mutex> lock(mChildrenObjectsMapLock);
    if(mChildrenObjectsMap->find(pid_alias) != mChildrenObjectsMap->end())
        cout << "Process name: " << pid_alias << " Id: " << mChildrenObjectsMap->at(pid_alias).get_id() << endl;
    else
        cout << "No such process name: " << pid_alias << endl;
}



bool ProcessScheduler::IsAnyProcessRunning()
{
    lock_guard<mutex> lock(mChildrenObjectsMapLock);
    return !mChildrenObjectsMap->empty();
}



void ProcessScheduler::CheckProcesses()
{
    {
        while(mChildrenObjectsMap->empty())
            this_thread::sleep(mProcessCheckInterval);
    }

    mCurCheckPidIt = mChildrenObjectsMap->begin();
    bool isFinised = false;
    while(!isFinised)
    {
        try
        {
            this_thread::interruption_point();
            {
                lock_guard<mutex> lock(mChildrenObjectsMapLock);
                /* map<string, child>::iterator could be instead of auto */
                for(auto it = mCurCheckPidIt; it != mChildrenObjectsMap->end(); ++it)
                {
                    mCurCheckPidIt = it;
                    mCurCheckPidAlias = it->first;
                    int proc_id = it->second.get_id();
#ifdef PM_DEBUG
                    cerr << "CheckProcesses() = " << "Process Name: " << mCurCheckPidAlias << " PID: " << it->second.get_id() << endl;
#endif

                    string s = "/proc/" + to_string(proc_id) + "/status";
                    path p(s);
                    if(0 == kill(proc_id, 0))
                    {
                        if(!exists(p))
                        {
                            s =  p.c_str();
                            s += " does not exist. CheckProcess FATAL ERROR.";
                            throw ProcessManagerException(s.c_str());
                        }
                        ifstream is(p.c_str(), ios::in);
                        s.erase();
                        getline(is, s);
                        cout << p.c_str() << " " << s << endl;

                    }
                    else
                    {
                        if(!exists(p))
                        {
                            s =  p.c_str();
                            s += " does not exist. CheckProcess FATAL ERROR.";
                            throw ProcessManagerException(s.c_str());
                        }
                    }
                }
#ifdef PM_DEBUG
                    cerr <<  endl;
#endif
                mCurCheckPidIt = mChildrenObjectsMap->begin();
            }
            this_thread::sleep(mProcessCheckInterval);
        }
        catch(system::system_error& e)
        {
#ifdef PM_DEBUG
            cerr << "CheckProcesses() = " << e.what() << endl;
#endif
            mChildrenObjectsMap->erase(mCurCheckPidAlias);
        }
        catch(thread_interrupted&)
        {
#ifdef PM_DEBUG
            cerr << "CheckProcesses() = " << "Thread interrupted" << endl;
#endif
            isFinised = true;
        }
        catch(ProcessManagerException& e)
        {
#ifdef PM_DEBUG
            cerr << "CheckProcesses() = " << e.what() << endl;
#endif
            mChildrenObjectsMap->erase(mCurCheckPidAlias);
            mCurCheckPidIt = mChildrenObjectsMap->begin();
        }
    }
}



void ProcessScheduler::_LaunchProcess(filesystem::path p, string arguments, string pid_alias)
{

    vector<string> args;
    vector<string> tmp = StringToVector(arguments);
    args.push_back(p.filename().c_str());
    args.insert(args.end(), tmp.begin(), tmp.end());

    string exec = find_executable_in_path(p.filename().c_str(), p.parent_path().c_str());
    context ctx;
    ctx.environment = self::get_environment();

    {
        lock_guard<mutex> lock(mChildrenObjectsMapLock);
        mChildrenObjectsMap->insert(pair<string, child>(pid_alias, launch(exec, args, ctx)));
    }

    mChildrenObjectsMap->at(pid_alias).wait();
}



void ProcessScheduler::_LaunchShell(string s , string pid_alias)
{
    context ctx;
    ctx.environment = self::get_environment();

    {
        lock_guard<mutex> lock(mChildrenObjectsMapLock);
        mChildrenObjectsMap->insert(pair<string, child>(pid_alias, launch_shell(s, ctx)));
    }
    mChildrenObjectsMap->at(pid_alias).wait();
}



} // End namespace Process

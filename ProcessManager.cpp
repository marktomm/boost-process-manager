#include <boost/process.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>

#include <deque>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <iostream>

#include "ProcessManager.h"

namespace Process
{

std::vector<std::string> StringToVector(std::string s)
{
    std::stringstream ss(s);
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::vector<std::string> args(begin, end);
    std::copy(args.begin(), args.end(), std::ostream_iterator<std::string>(std::cout, "\n"));

    return args;
}


ProcessScheduler::ProcessScheduler()
    :
      mSelf(boost::process::self::get_instance()),
      mChildrenObjectsMap(new std::map<std::string, boost::process::child>),
      mCurTerminatigPidAlias(""),
      mProcessCheckInterval(1000),
      mIsProcessCheckInProgress(false)
{

}

ProcessScheduler::ProcessScheduler( boost::filesystem::path p, std::string arguments, std::string pid_alias)
    :
      mSelf(boost::process::self::get_instance()),
      mChildrenObjectsMap(new std::map<std::string, boost::process::child>),
      mCurTerminatigPidAlias(""),
      mProcessCheckInterval(1000),
      mIsProcessCheckInProgress(false)
{
    LaunchProcess(p, arguments, pid_alias);
}

ProcessScheduler::ProcessScheduler( std::string s , std::string pid_alias)
    :
      mSelf(boost::process::self::get_instance()),
      mChildrenObjectsMap(new std::map<std::string, boost::process::child>),
      mCurTerminatigPidAlias(""),
      mProcessCheckInterval(1000),
      mIsProcessCheckInProgress(false)
{
    LaunchShell(s, pid_alias);
}

ProcessScheduler::~ProcessScheduler()
{
    TerminateAllProcesses();
    delete mChildrenObjectsMap;
}

void ProcessScheduler::LaunchProcess(boost::filesystem::path p, std::string arguments, std::string pid_alias)
{

    std::vector<std::string> args;
    std::vector<std::string> tmp = StringToVector(arguments);
    args.push_back(p.filename().c_str());
    args.insert(args.end(), tmp.begin(), tmp.end());

    std::string exec = boost::process::find_executable_in_path(p.filename().c_str(), p.parent_path().c_str());
    boost::process::context ctx;
    ctx.environment = boost::process::self::get_environment();

    {
        boost::lock_guard<boost::mutex> lock(mChildrenObjectsMapLock);
        mChildrenObjectsMap->insert(std::pair<std::string, boost::process::child>(pid_alias, boost::process::launch(exec, args, ctx)));
    }

    if(mIsProcessCheckInProgress == false)
    {
#ifdef PM_DEBUG
        std::cout << "Starting processes check thread" << std::endl;
#endif
        mThreadCheckProcesses = boost::thread(&ProcessScheduler::CheckProcesses, this);
        mIsProcessCheckInProgress = true;
    }
}

void ProcessScheduler::LaunchShell(std::string s , std::string pid_alias)
{
    boost::process::context ctx;
    ctx.environment = boost::process::self::get_environment();

    {
        boost::lock_guard<boost::mutex> lock(mChildrenObjectsMapLock);
        mChildrenObjectsMap->insert(std::pair<std::string, boost::process::child>(pid_alias, boost::process::launch_shell(s, ctx)));
    }

    if(mIsProcessCheckInProgress == false)
    {
        mThreadCheckProcesses = boost::thread(&ProcessScheduler::CheckProcesses, this);
        mIsProcessCheckInProgress = true;
    }
}

void ProcessScheduler::TerminateProcess(std::string pid_alias)
{
    boost::lock_guard<boost::mutex> lock(mChildrenObjectsMapLock);
    mCurTerminatigPidAlias = pid_alias;
    try
    {
        /*boost::process::status s =*/  /*mChildrenObjectsMap->at(pid_alias).wait();*/

        /*
        *  Use status if needed
        */

        mChildrenObjectsMap->at(pid_alias).terminate();

    }
    catch(boost::system::system_error& e)
    {
        std::cerr << "TerminateProcess() = " << e.what() << std::endl;
        mChildrenObjectsMap->erase(mCurTerminatigPidAlias);
    }
}

void ProcessScheduler::TerminateAllProcesses()
{
    mThreadCheckProcesses.interrupt();
    bool isFinished = false;
    while(!isFinished)
    {
        try
        {
            /* std::map<std::string, boost::process::child*>::iterator could be instead of auto */
            for(auto it = mChildrenObjectsMap->begin(); it != mChildrenObjectsMap->end(); ++it)
            {
                mCurTerminatigPidAlias = it->first;
                /*boost::process::status s =*/ /*it->second.wait();*/

                /*
                *  Use status if needed
                */

                it->second.terminate();
                mChildrenObjectsMap->erase(it);
            }
            isFinished=true;
        }
        catch(boost::system::system_error& e)
        {
            std::cerr << "TerminateAllProcesses() = " << e.what() << std::endl;
            mChildrenObjectsMap->erase(mCurTerminatigPidAlias);
        }
    }
}


void ProcessScheduler::ViewProcessPids()
{
    try
    {
        boost::lock_guard<boost::mutex> lock(mChildrenObjectsMapLock);
        /* std::map<std::string, boost::process::child>::iterator could be instead of auto */
        if(!mChildrenObjectsMap->empty())
            for(auto it = mChildrenObjectsMap->begin(); it != mChildrenObjectsMap->end(); ++it)
            {
                std::cout << "Process id: " << it->second.get_id() << " Name: " << it->first << std::endl;
            }
        else
            std::cerr << "Process list empty" << std::endl;
    }
    catch(boost::system::system_error& e)
    {
        std::cerr << "ViewProcessPids() = " << e.what() << std::endl;
    }
}

void ProcessScheduler::ViewProcessPid(std::string pid_alias)
{
    boost::lock_guard<boost::mutex> lock(mChildrenObjectsMapLock);
    if(mChildrenObjectsMap->find(pid_alias) != mChildrenObjectsMap->end())
        std::cout << "Process name: " << pid_alias << " Id: " << mChildrenObjectsMap->at(pid_alias).get_id() << std::endl;
    else
        std::cout << "No such process name: " << pid_alias << std::endl;
}

bool ProcessScheduler::IsAnyProcessRunning()
{
    boost::lock_guard<boost::mutex> lock(mChildrenObjectsMapLock);
    return !mChildrenObjectsMap->empty();
}


void ProcessScheduler::CheckProcesses()
{
    mCurCheckPidIt = mChildrenObjectsMap->begin();
    bool isFinised = false;
    while(!isFinised)
    {
        try
        {
            boost::this_thread::interruption_point();
            {
                boost::lock_guard<boost::mutex> lock(mChildrenObjectsMapLock);
                /* std::map<std::string, boost::process::child>::iterator could be instead of auto */
                for(auto it = mCurCheckPidIt; it != mChildrenObjectsMap->end(); ++it)
                {
#ifdef PM_DEBUG
                    std::cerr << "CheckProcesses() = " << "Process Name: " << it->first << " PID: " << it->second.get_id() << std::endl;
#endif
                    mCurCheckPidIt = it;
                    mCurCheckPidAlias = it->first;
                    it->second.get_id();
                }
#ifdef PM_DEBUG
                    std::cerr <<  std::endl;
#endif
                mCurCheckPidIt = mChildrenObjectsMap->begin();
            }
            boost::this_thread::interruption_point();
            boost::this_thread::sleep(mProcessCheckInterval);
        }
        catch(boost::system::system_error& e)
        {
#ifdef PM_DEBUG
            std::cerr << "CheckProcesses() = " << e.what() << std::endl;
#endif
            mChildrenObjectsMap->erase(mCurCheckPidAlias);
        }
        catch(boost::thread_interrupted&)
        {
#ifdef PM_DEBUG
            std::cerr << "CheckProcesses() = " << "Thread interrupted" << std::endl;
#endif
            isFinised = true;
        }
    }
}


}

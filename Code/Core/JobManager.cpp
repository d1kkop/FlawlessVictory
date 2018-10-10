#include "PCH.h"
#include "JobManager.h"
#include "Functions.h"
#include "Algorithm.h"
#include "OSLayer.h"
using namespace std;

namespace fv
{
    // ------------- Job ------------------------------------------------------------------------------------------------------------

    JobState Job::state() const
    {
        return m_State;
    }

    void Job::wait()
    {
        // Peek done, if so no need to enter state lock.
        if ( m_State == JobState::Done || m_State == JobState::Cancelled )
            return;
        // While there are other jobs in queue, process them first to not stall the thread and avoid waiting for a child job
        // that will never run.
        m_Jm->processQueue();
        // Wait on the requested job now.
        unique_lock<Mutex> lk(m_StateMutex);
        m_NumWaiters++;
        while ( !(m_State == JobState::Done || m_State == JobState::Cancelled) )
        {
            m_StateSignal.wait( lk );
        }
        m_NumWaiters--;
    }

    void Job::waitAndFree()
    {
        wait();
        doFree();
    }

    bool Job::cancel()
    {
        return m_Jm->cancelJob(this);
    }

    bool Job::cancelAndFree()
    {
        bool bRes = cancel();
        waitAndFree();
        return bRes;
    }

    void Job::finishWith(JobState newState)
    {
        {
            scoped_lock lk(m_StateMutex);
            m_State = newState;
            if ( m_NumWaiters > 0 )
            {
                m_StateSignal.notify_all();
            }
        }
        if  ( m_OnDoneOrCancelled ) m_OnDoneOrCancelled( this );
        if ( m_AutoFree ) doFree();
    }

    void Job::doFree()
    {
        m_Jm->freeJob(this);
    }

    // ------------- JobManager ------------------------------------------------------------------------------------------------------

    JobManager::JobManager():
        ObjectManager<Job>(16, true)
    {
        u32 numThreads = std::thread::hardware_concurrency();
        m_Threads.resize( numThreads );
        for ( u32 i = 0; i < numThreads; i++ )
        {
            m_Threads[i] = Thread( [this, i]()
            {
                set_terminate([]()
                {
                    LOGC("Worker threads encountered exception.");
                    abort();
                });
                OSSetThreadName(Format("JobManagerThread %d", i).c_str());
                threadLoop();
            });
        }
    }

    JobManager::~JobManager()
    {
        {
            scoped_lock lk(m_QueueMutex);
            m_IsClosing = true;
            m_NumThreadsSuspended = 0;
            m_ThreadSuspendSignal.notify_all();
        }
        for ( auto& t : m_Threads )
        {
            if ( t.joinable() ) t.join();
        }
        // If still had jobs in queue, process them ST
        processQueue();
    }

    Job* JobManager::addJob(const Function<void ()>& cb, bool autoFree, const Function<void (Job*)>& onDoneOrCancelled)
    {
        assert( cb );

        // ObjectManager IS thread safe created
        Job* job = newObject();
        
        job->m_Jm = this;
        job->m_State = JobState::Scheduled;
        job->m_Cb = cb;
        job->m_OnDoneOrCancelled = onDoneOrCancelled;
        job->m_NumWaiters = 0;
        job->m_AutoFree = autoFree;

    #if FV_USEJOBSYSTEM
        scoped_lock lk(m_QueueMutex);
        m_GlobalQueue.emplace_back( job );
        if ( m_NumThreadsSuspended > 0 )
        {
            m_NumThreadsSuspended--;
            m_ThreadSuspendSignal.notify_one();
        }
    #else
        if ( cb ) cb();
        job->finishWith(JobState::Done);
        if ( onDoneOrCancelled ) onDoneOrCancelled( job );
    #endif

        return job;
    }

    u32 JobManager::numThreads() const
    {
        return (u32)m_Threads.size();
    }

    void JobManager::freeJob(Job* job)
    {
        assert(job);
        job->wait();
        job->m_Cb = nullptr; // Set these to null to ensure shared embedded resources are unreffed.
        job->m_OnDoneOrCancelled = nullptr; 
        // ObjectManager is thread safe created
        freeObject(job);
    }

    bool JobManager::cancelJob(Job* job)
    {
        assert(job);
        unique_lock<Mutex> lk(m_QueueMutex);
        if ( job->m_State == JobState::Scheduled )
        {
            Remove( m_GlobalQueue, job );
            lk.unlock();
            job->finishWith( JobState::Cancelled );
            return true;
        }
        return false;
    }

    void JobManager::threadLoop()
    {
        while ( true )
        {
            unique_lock<Mutex> lk( m_QueueMutex );
            if ( m_IsClosing )
            {
                break;
            }
            if ( m_GlobalQueue.size() )
            {
                popAndProcessJob( lk );
            }
            else
            {
                m_NumThreadsSuspended++;
                m_ThreadSuspendSignal.wait(lk);
            }
        }
    }

    void JobManager::processQueue()
    {
        unique_lock<Mutex> lk(m_QueueMutex);
        while ( m_GlobalQueue.size() )
        {
            popAndProcessJob(lk);
            lk.lock();
        }
    }

    void JobManager::popAndProcessJob(unique_lock<Mutex>& lk)
    {
        Job* job = m_GlobalQueue.front();
        m_GlobalQueue.pop_front();
        // Should not need jobState lock for this state swap
        job->m_State = JobState::InProgress;
        lk.unlock();
        assert(job->m_Cb);
        if ( job->m_Cb ) job->m_Cb();
        job->finishWith(JobState::Done);
    }


    JobManager* g_JobManager = nullptr;
    JobManager* jobManager() { return CreateOnce(g_JobManager); }
    void deleteJobManager() { delete g_JobManager; g_JobManager=nullptr; }

}
#include "JobManager.h"
#include "../Core/Functions.h"
#include "../Core/OSLayer.h"
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
        // While there are other jobs in queue, process them first to not stall the thread and avoid waiting for a child job
        // that will never run.
        m_Jm->processQueue();
        // Wait on the requested job now.
        unique_lock<Mutex> lk(m_StateMutex);
        while ( !(m_State == JobState::Done || m_State == JobState::Cancelled) )
        {
            m_StateSignal.wait( lk );
        }
    }

    void Job::free()
    {
        m_Jm->freeJob(this);
    }

    void Job::waitAndFree()
    {
        wait();
        free();
    }

    bool Job::cancel()
    {
        return m_Jm->cancelJob(this);
    }

    bool Job::cancelAndFree()
    {
        bool bRes = cancel();
        free();
        return bRes;
    }

    void Job::finishWith(JobState newState)
    {
        scoped_lock lk(m_StateMutex);
        m_State = newState;
        if ( m_NumWaiters > 0 )
        {
            m_StateSignal.notify_all();
        }
    }

    // ------------- JobManager ------------------------------------------------------------------------------------------------------

    JobManager::JobManager()
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

    Job* JobManager::addJob( const Function<void (Job * )>& cb )
    {
        // ObjectManager is not thread safe
        Job* job;
        {
            scoped_lock lk(m_ObjectManagerMutex);
            job = newObject();
        }
        
        job->m_Jm = this;
        job->m_State = JobState::Scheduled;
        job->m_Cb = cb;
        job->m_NumWaiters = 0;

        scoped_lock lk(m_QueueMutex);
        m_GlobalQueue.emplace_back( job );
        if ( m_NumThreadsSuspended > 0 )
        {
            m_NumThreadsSuspended--;
            m_ThreadSuspendSignal.notify_all();
        }

        return job;
    }

    void JobManager::freeJob(Job* job)
    {
        assert(job);
        // ObjectManager is not thread safe
        {
            scoped_lock lk(m_ObjectManagerMutex);
            freeObject(job);
        }
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
        }
    }

    void JobManager::popAndProcessJob(unique_lock<Mutex>& lk)
    {
        Job* job = m_GlobalQueue.front();
        m_GlobalQueue.pop_front();
        // Should not need jobState lock for this state swap
        job->m_State = JobState::InProgress;
        lk.unlock();
        job->m_Cb(job);
        job->finishWith(JobState::Done);
    }


    JobManager* g_JobManager = nullptr;
    JobManager* jobManager() { return CreateOnce(g_JobManager); }
    void deleteJobManager() { delete g_JobManager; g_JobManager=nullptr; }

}
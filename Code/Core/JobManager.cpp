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
        jobManager()->processQueue();
        // Wait on the requested job now.
        unique_lock<Mutex> lk(m_StateMutex);
        m_NumWaiters++;
        while ( !(m_State == JobState::Done || m_State == JobState::Cancelled) )
        {
            m_StateSignal.wait( lk );
        }
        assert(m_NumWaiters>0);
        m_NumWaiters--;
    }

    bool Job::cancel()
    {
        return jobManager()->cancelJob(this);
    }

    void Job::finishWith(JobState newState)
    {
        assert( newState == JobState::Cancelled || newState == JobState::Done );
        if ( m_OnDoneOrCancelled ) m_OnDoneOrCancelled( this );
        {
            scoped_lock lk(m_StateMutex);
            m_State = newState;
            if ( m_NumWaiters > 0 )
            {
                m_StateSignal.notify_all();
            }
        }
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
            m_ThreadSuspendSignal.notify_all();
        }
        for ( auto& t : m_Threads )
        {
            if ( t.joinable() ) t.join();
        }
        assert(m_NumThreadsSuspended == 0);
        // If still had jobs in queue, process them ST
        processQueue();
    }

    M<Job> JobManager::addJob(const Function<void ()>& cb, const Function<void (Job*)>& onDoneOrCancelled)
    {
        assert( cb );

        // ObjectManager IS thread safe created
        M<Job> job =  M<Job>(newObject(), freeJob);
        
        job->m_Cb = cb;
        job->m_OnDoneOrCancelled = onDoneOrCancelled;
        assert( job->m_State == JobState::Scheduled );
        assert( job->m_NumWaiters == 0 );

    #if FV_USEJOBSYSTEM
        scoped_lock lk(m_QueueMutex);
        m_GlobalQueue.emplace_back( job );
        if ( m_NumThreadsSuspended > 0 )
        {
            m_ThreadSuspendSignal.notify_one();
        }
    #else
        cb();
        job->finishWith(JobState::Done);
    #endif

        return job;
    }

    u32 JobManager::numThreads() const
    {
        return (u32)m_Threads.size();
    }

    bool JobManager::cancelJob(Job* job)
    {
        assert(job);
        unique_lock<Mutex> lk(m_QueueMutex);
        if ( job->m_State == JobState::Scheduled )
        {
            Remove_if( m_GlobalQueue, [job](auto& mjob) { return mjob.get()==job; } );
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
            unique_lock<Mutex> lk(m_QueueMutex);
            if ( m_IsClosing ) return;
            M<Job> job;
            if ( extractJob(job) )
            {
                lk.unlock();
                assert(job->m_Cb);
                if ( job->m_Cb ) job->m_Cb();
                job->finishWith(JobState::Done);
            }
            else
            {
                m_NumThreadsSuspended++;
                m_ThreadSuspendSignal.wait(lk);
                assert(m_NumThreadsSuspended > 0);
                m_NumThreadsSuspended--;
            }
        }
    }

    void JobManager::processQueue()
    {
        unique_lock<Mutex> lk(m_QueueMutex);
        while ( !m_GlobalQueue.empty() )
        {
            M<Job> job;
            if ( extractJob(job) )
            {
                lk.unlock();
                assert(job->m_Cb);
                if ( job->m_Cb ) job->m_Cb();
                job->finishWith(JobState::Done);
                lk.lock();
            }
            else break;
        }
    }

    bool JobManager::extractJob(M<Job>& job)
    {
        if ( m_GlobalQueue.size() )
        {
            job = m_GlobalQueue.front();
            m_GlobalQueue.pop_front();
            // Should not need state lock for this. Has global queue mutex, so state cannot changed to cancelled.
            job->m_State = JobState::InProgress;
            return true;
        }
        return false;
    }

    void JobManager::freeJob(Job* job)
    {
        assert(job && (job->m_State == JobState::Cancelled || job->m_State == JobState::Done) && job->m_NumWaiters==0);
        // ObjectManager is thread safe created
        jobManager()->freeObject(job);
    }

    JobManager* g_JobManager = nullptr;
    JobManager* jobManager() { return CreateOnce(g_JobManager); }
    void deleteJobManager() { delete g_JobManager; g_JobManager=nullptr; }

}
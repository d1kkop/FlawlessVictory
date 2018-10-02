#include "JobSystem.h"
#include "../Core/Functions.h"
#include "../Core/LogManager.h"
using namespace std;


namespace fv
{
    // ------------ Job --------------------------------------------------------------------------------

    Job::Job(const Function<void ()>& cb):
        m_Cb(cb)
    {
    }

    void Job::wait()
    {
        std::unique_lock lk(m_StateMutex);
        while ( !(m_State == JobState::Cancelled || m_State == JobState::Done ) )
        {
            m_StateSignal.wait( m_StateMutex );
        }
    }

    // ------------ WorkerThread --------------------------------------------------------------------------------

	WorkerThread::WorkerThread(JobSystem& js):
		m_Js(js),
		m_Closing(false)
	{
	}

	WorkerThread::~WorkerThread()
	{
		stop();
	}

	void WorkerThread::start(const char* name)
	{
        m_Thread = Thread( [this]()
        {
            set_terminate([]()
            {
                LOGC("Worker threas encountered exception.");
                abort();
            });
             threadLoop();
        });
	}

	void WorkerThread::stop()
	{
		if ( m_Thread.joinable() )
		{
			m_Thread.join();
		}
	}

    void WorkerThread::threadLoop()
    {
        while ( true )
        {
            unique_lock<Mutex> lk(m_Js.getMutex());
            if ( m_Js.isClosing() )
            {
                break;
            }
            Job job;
            if ( m_Js.extractJob(job) )
            {
                lk.unlock();
            #if FV_TRACEJOBSYSTEM
                LOG("Thread %s starts to execute job.", m_Thread.name().c_str());
            #endif
                job.m_Cb();
            }
            else
            {
                m_Js.suspend(lk);
            }
        }
    }

    // ------------ JobSystem --------------------------------------------------------------------------------

	JobSystem::JobSystem():
		m_Closing(false),
		m_NumSuspendedThreads(0)
	{
        u32 numThreads = std::thread::hardware_concurrency();
		for (u32 i = 0; i < numThreads ; i++)
		{
			m_WorkerThreads.push_back( make_unique<WorkerThread>(*this) );
			m_WorkerThreads.back()->start( Format("JobSystemThread %d", i).c_str() );
		}
	}

	JobSystem::~JobSystem()
	{
		stop();
	}

	Job* JobSystem::addJob(const Function<void(Job*)>& cb)
	{
		#if !FV_MT
			cb();
			return nullptr;
		#endif

		#if FV_TRACEJOBSYSTEM
			LOG( "Thread %s locks.", m_Thread.name().c_str() );
		#endif

        newObject();

		m_JobsMutex.lock();
		m_GlobalQueue.push ( { cb } );
		if ( m_NumSuspendedThreads > 0 )
		{
			m_QueueCv.notify_one();
			m_NumSuspendedThreads--;
		}
		m_JobsMutex.unlock();

		#if FV_TRACEJOBSYSTEM
			LOG( "Thread %s unlocks.", m_Thread.name().c_str() );
		#endif
	}

	bool JobSystem::extractJob(Job& job)
	{
		if ( !m_GlobalQueue.empty() )
		{
			job = m_GlobalQueue.front();
			m_GlobalQueue.pop();
			return true;
		}
		return false;
	}

	void JobSystem::suspend(unique_lock<Mutex>& ul)
	{
		#if FV_TRACEJOBSYSTEM
			LOG( "Thread %s suspends -> loses lock.", m_Thread.name().c_str() );
		#endif
		m_NumSuspendedThreads++;
		m_QueueCv.wait( ul );
	}

    void JobSystem::growJobs()
    {

    }

    Job* JobSystem::getNewJob()
    {

    }

    void JobSystem::stop()
	{
		m_JobsMutex.lock();
		m_Closing = true;
		m_QueueCv.notify_all();
		m_JobsMutex.unlock();
		m_WorkerThreads.clear();
		scoped_lock lk(m_JobsMutex);
		while ( !m_GlobalQueue.empty() ) m_GlobalQueue.pop();
	}

}
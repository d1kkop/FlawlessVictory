#pragma once

#pragma once
#include "../Core/Common.h"
#include "../Core/Thread.h"
#include "../Core/ObjectManager.h"

namespace fv
{
    enum JobState
    {
        Idle,
        Scheduled,
        InProgress,
        Cancelled,
        Done
    };

    class Job: public Object
    {
    public:
        FV_TS FV_DLL JobState state() const;
        FV_TS FV_DLL void wait();
        FV_TS FV_DLL void free();
        FV_TS FV_DLL void waitAndFree();

        // Will only work if job was not yet started. This will attempt to remove it from the job queue.
        // Returns true if was actually removed from queue. False otherwise.
        // The job is not yet freed after this. 'free()' must still be called.
        FV_TS FV_DLL bool cancel();

        // See above for cancel. Calls free automatically afterwards.
        FV_TS FV_DLL bool cancelAndFree();

    private:
        FV_TS void finishWith(JobState newState);

        class JobManager* m_Jm;
        Function<void (Job*)> m_Cb;
        Atomic<JobState> m_State = JobState::Idle;
        Mutex m_StateMutex;
        CondVar m_StateSignal;
        u32 m_NumWaiters = 0;

        friend class JobManager;
        friend class ObjectManager<Job>;
    };

    class JobManager: public ObjectManager<Job>
    {
    public:
        FV_DLL JobManager();
        FV_TS FV_DLL Job* addJob( const Function<void (Job*)>& cb );

    private:
        FV_TS void freeJob(Job* job);
        FV_TS bool cancelJob(Job* job);
        FV_TS void threadLoop();
        FV_TS void processQueue();
        FV_TS void popAndProcessJob(std::unique_lock<Mutex>& lk);

        bool m_IsClosing = false;
        Mutex m_ObjectManagerMutex;
        Mutex m_QueueMutex;
        Deck<Job*> m_GlobalQueue;
        Vector<Thread> m_Threads;
        CondVar m_ThreadSuspendSignal;
        u32 m_NumThreadsSuspended = 0;

        friend class Job;
    };


    FV_DLL JobManager* jobManager();
    FV_DLL void deleteJobManager();
}
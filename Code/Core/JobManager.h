#pragma once
#include "PCH.h"
#include "Common.h"
#include "Thread.h"
#include "Functions.h"
#include "SparseArray.h"

namespace fv
{
    enum JobState
    {
        Scheduled,
        InProgress,
        Cancelled,
        Done
    };

    class Job: public Object
    {
    public:
        // Peeks job state.
        FV_TS FV_DLL JobState state() const;

        // If job was already finished or cancelled, returns immediately.
        FV_TS FV_DLL void wait();

        // Will only work if job was not yet started. This will attempt to remove it from the job queue.
        // Returns true if was actually removed from queue. False otherwise.
        FV_TS FV_DLL bool cancel();

    private:
        FV_TS void finishWith(JobState newState);

        Function<void ()> m_Cb;
        Function<void (Job*)> m_OnDoneOrCancelled;
        Atomic<JobState> m_State = JobState::Scheduled;
        Mutex m_StateMutex;
        CondVar m_StateSignal;
        u32 m_NumWaiters = 0;
        u32 m_ExecOn = -1;

        friend class JobManager;
        friend class SparseArray<Job>;
    };

    class JobManager: public SparseArray<Job>
    {
    public:
        FV_DLL JobManager();
        FV_DLL ~JobManager();

        FV_TS FV_DLL M<Job> addJob( const Function<void ()>& cb, const Function<void (Job*)>& onDoneOrCancelled = Function<void (Job*)>() );
        FV_TS FV_DLL M<Job> addJobOn( u32 tIdx, const Function<void ()>& cb, const Function<void (Job*)>& onDoneOrCancelled = Function<void (Job*)>() );
        FV_TS FV_DLL u32 numThreads() const;
        FV_TS FV_DLL u32 threadIdToIdx() const;

    private:
        FV_TS bool cancelJob(Job* job);
        FV_TS void threadLoop();
        FV_TS void processQueue();
        bool extractJob(M<Job>& job);
        static void freeJob(Job* job);

        bool m_IsClosing = false;
        Mutex m_QueueMutex;
        Deck<M<Job>> m_GlobalQueue;
        Vector<Thread> m_Threads;
        CondVar m_ThreadSuspendSignal;
        u32 m_NumThreadsSuspended = 0;

        friend class Job;
    };


    FV_DLL JobManager* jobManager();
    FV_DLL void deleteJobManager();

    inline void ParallelForPrepare(i32 size, i32&s, i32& nt, i32& pt, i32& ofs)
    {
        s  = size;
        nt = jobManager()->numThreads();
        pt = (s+(nt-1)) / nt;
        ofs = 0;
    }

    template <class T, class C, class CB>
    void ParallelComponentFor(const C& collection, const CB& cb)
    {
        i32 kRemaining, kThreads, kPerThread, ofs;
        ParallelForPrepare( (i32)collection.size(), kRemaining, kThreads, kPerThread, ofs );
        M<Job> jobs[64];
        assert(kThreads<=64);
        i32 i;
        for ( i=0; i<kThreads && kRemaining>0; ++i )
        {
            u32 count = Min(kRemaining, kPerThread);
            jobs[i] = jobManager()->addJob([=,&collection,&cb]()
            {
                for ( u32 j=ofs; j<ofs+count; ++j )
                {
                    auto& compArray = collection[j];
                    for ( u32 k=0; k<compArray.size; ++k )
                    {
                        T* comp = (T*)((char*)compArray.elements + k*compArray.compSize);
                        if ( comp->inUse() )
                            cb ( *comp );
                    }
                }
            });
            kRemaining -= kPerThread;
            ofs += kPerThread;
        }
        for ( i32 a=0; a<i; ++a ) jobs[a]->wait();
    }


    template <class C, class CB>
    void ParallelFor(const C& collection, const CB& cb)
    {
        i32 kRemaining, kThreads, kPerThread, ofs;
        ParallelForPrepare((i32)collection.size(), kRemaining, kThreads, kPerThread, ofs);
        M<Job> jobs[64];
        assert(kThreads<=64);
        i32 i;
        for ( i=0; i<kThreads && kRemaining>0; ++i )
        {
            u32 count = Min(kRemaining, kPerThread);
            jobs[i] = jobManager()->addJob([=,&collection,&cb]()
            {
                for ( u32 j=ofs; j<ofs+count; ++j )
                {
                    cb( collection[j], i );
                }
            });
            kRemaining -= kPerThread;
            ofs += kPerThread;
        }
        for ( i32 a=0; a<i; ++a ) jobs[a]->wait();
    }
}
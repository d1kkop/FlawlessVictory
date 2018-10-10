#pragma once
#include "PCH.h"
#include "Common.h"
#include "Thread.h"
#include "Functions.h"
#include "ObjectManager.h"

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
        FV_TS FV_DLL JobState state() const;
        FV_TS FV_DLL void wait();
        FV_TS FV_DLL void waitAndFree();

        // Will only work if job was not yet started. This will attempt to remove it from the job queue.
        // Returns true if was actually removed from queue. False otherwise.
        // The job is not yet freed after this. 'free()' must still be called.
        FV_TS FV_DLL bool cancel();

        // See above for cancel. Calls free automatically afterwards.
        FV_TS FV_DLL bool cancelAndFree();

    private:
        FV_TS void finishWith(JobState newState);
        FV_TS void doFree();

        class JobManager* m_Jm;
        Function<void ()> m_Cb;
        Function<void (Job*)> m_OnDoneOrCancelled;
        Atomic<JobState> m_State = JobState::Scheduled;
        Mutex m_StateMutex;
        CondVar m_StateSignal;
        u32 m_NumWaiters = 0;
        bool m_AutoFree  = false;

        friend class JobManager;
        friend class ObjectManager<Job>;
    };

    class JobManager: public ObjectManager<Job>
    {
    public:
        FV_DLL JobManager();
        FV_DLL ~JobManager();
        FV_TS FV_DLL Job* addJob( const Function<void ()>& cb, bool autoFree = false, const Function<void (Job*)>& onDoneOrCancelled = Function<void (Job*)>() );
        FV_TS FV_DLL u32 numThreads() const;

    private:
        FV_TS void freeJob(Job* job);
        FV_TS bool cancelJob(Job* job);
        FV_TS void threadLoop();
        FV_TS void processQueue();
        FV_TS void popAndProcessJob(std::unique_lock<Mutex>& lk);

        bool m_IsClosing = false;
        Mutex m_QueueMutex;
        Deck<Job*> m_GlobalQueue;
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

    inline void ParallelForFinish(Job** jobs, i32 i)
    {
        for ( ; i>=0; --i )
        {
            jobs[i]->waitAndFree();
        }
    }

    template <class T, class C, class CB>
    void ParallelComponentFor(const C& collection, const CB& cb)
    {
        i32 s, nt, pt, ofs;
        ParallelForPrepare( (i32)collection.size(), s, nt, pt, ofs );
        Job* jobs[64];
        assert(nt<=64);
        i32 i;
        for ( i=0; i<nt && s>0; ++i )
        {
            u32 count = Min(s, pt);
            jobs[i] = jobManager()->addJob([=]()
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
            s -= pt;
            ofs += pt;
        }
        i--;
        ParallelForFinish(jobs, i);
    }


    template <class C, class CB>
    void ParallelFor(const C& collection, const CB& cb)
    {
        i32 s, nt, pt, ofs;
        ParallelForPrepare((i32)collection.size(), s, nt, pt, ofs);
        Job* jobs[64];
        assert(nt<=64);
        i32 i;
        for ( i=0; i<nt && s>0; ++i )
        {
            u32 count = Min(s, pt);
            jobs[i] = jobManager()->addJob([=]()
            {
                for ( u32 j=ofs; j<ofs+count; ++j )
                {
                    cb( collection[j] );
                }
            });
            s -= pt;
            ofs += pt;
        }
        i--;
        ParallelForFinish(jobs, i);
    }
}
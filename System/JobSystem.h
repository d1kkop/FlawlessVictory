#pragma once
#include "../Core/Common.h"
#include "../Core/Thread.h"
#include "../Scene/ObjectManager.h"


namespace fv
{
    enum JobState
    {
        Scheduled,
        InProgress,
        Cancelled,
        Done
    };

    class Job
    {
    public:
        Job(const Function<void ()>& cb);
        void wait();

    private:
        void finishWith(JobState state);

        Function<void (Job*)> m_Cb;
        Atomic<JobState> m_State;
        Mutex m_StateMutex;
        CondVar m_StateSignal;

        friend class JobSystem;
    };

	class WorkerThread
	{
	public:
		WorkerThread(class JobSystem& js);
		~WorkerThread();
        void start(const char* name);
        void stop();

    private:
        void threadLoop();

		Thread m_Thread;
		class JobSystem& m_Js;
		volatile bool m_Closing;
	};


	class JobSystem: public ObjectManager<Job>
	{
	public:
		JobSystem();
		~JobSystem();

		Job* addJob( const Function<void ()>& cb );
		void stop();
		bool isClosing() const volatile { return m_Closing; }

	private:
        bool extractJob(Function<void ()>& job);
        Mutex& getMutex() { return m_JobsMutex; }
        void suspend(std::unique_lock<Mutex>& ul);

		volatile bool m_Closing;
		Mutex m_JobsMutex;
		u32 m_NumSuspendedThreads;
		Queue<Job*> m_GlobalQueue;
		CondVar m_QueueCv;
		Vector<U<WorkerThread>> m_WorkerThreads;

		friend class WorkerThread;
	};

}
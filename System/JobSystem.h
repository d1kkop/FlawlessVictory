#pragma once
#include "../Core/Common.h"
#include "../Core/Thread.h"


namespace fv
{
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


	class JobSystem
	{
	public:
		JobSystem();
		~JobSystem();

		void addJob( const Function<void ()>& cb );
		void stop();
		bool isClosing() const volatile { return m_Closing; }

	private:
        bool extractJob(Function<void ()>& job);
        Mutex& getMutex() { return m_JobsMutex; }
        void suspend(std::unique_lock<Mutex>& ul);

	private:
		volatile bool m_Closing;
		Mutex m_JobsMutex;
		u32 m_NumSuspendedThreads;
		Queue<Function<void ()>> m_GlobalQueue;
		CondVar m_QueueCv;
		Array<U<WorkerThread>> m_WorkerThreads;

		friend class WorkerThread;
	};

}
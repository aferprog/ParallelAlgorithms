#pragma once
#include <queue>
#include <functional>
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
//
//struct task_info {
//	std::function<void()> task;
//	bool bInfinity = false;
//	bool bNecessary = true;
//};

struct mte_thread {
	std::thread thread;
	std::atomic_bool is_ready = false;
};

using PoolOfThreads = std::vector<mte_thread>;

class MultiThreadEngine
{
	friend void work(MultiThreadEngine* mte, mte_thread& thrd);

	PoolOfThreads threads;
	std::queue<std::function<void()>> tasks;
	std::atomic_bool bShouldFinish = false, bShouldStop = false;
	mutable std::recursive_mutex queue_mutex;
	std::condition_variable_any cv;

	mutable std::mutex wait_mutex;
	std::condition_variable wait_cv;

public:
	size_t getTasksCount() const;
	size_t getThreadsCount() const;
	bool shouldFinish() const;
	bool shouldStop() const;
	void stop();
	void wait();
	void addTask(std::function<void()> task);
	explicit MultiThreadEngine(unsigned int threads_count);
	MultiThreadEngine(const MultiThreadEngine& other) = delete;
	~MultiThreadEngine();
};
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

class MultyThreadEngine
{
	friend void work(MultyThreadEngine* mte);

	std::vector<std::thread> threads;
	std::queue<std::function<void()>> tasks;
	mutable std::recursive_mutex queue_mutex;
	std::atomic_bool bShouldFinish = false, bShouldStop = false;
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
	MultyThreadEngine();
	MultyThreadEngine(unsigned int threads_count);
	~MultyThreadEngine();
};
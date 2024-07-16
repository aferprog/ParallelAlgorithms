#include "MultiThreadEngine.h"

static void work(MultiThreadEngine* mte, mte_thread& thr){
	while (true) {
		std::function<void()> task;
		{
			std::unique_lock<std::recursive_mutex> lock(mte->queue_mutex);
			mte->cv.wait(lock, [mte]() {
				return mte->shouldStop() || (mte->tasks.empty() && mte->shouldFinish()) || !mte->tasks.empty();
				});

			if (mte->shouldStop() || mte->getTasksCount() == 0ull)
				return;
			thr.is_ready = false;
			task = std::move(mte->tasks.front());
			mte->tasks.pop();
		}
		task();
		thr.is_ready = true;
		mte->wait_cv.notify_all();
	}
}
void MultiThreadEngine::addTask(std::function<void()> task) {
	if (bShouldFinish || bShouldStop)
		throw std::runtime_error("Adding new tasks during stopping is forbidden.");
	{
		std::lock_guard lock(queue_mutex);
		tasks.push(task);
	}
	cv.notify_one();
}

bool MultiThreadEngine::shouldFinish() const {
	return bShouldFinish.load();
}
bool MultiThreadEngine::shouldStop() const {
	return bShouldStop.load();
}

void MultiThreadEngine::stop()
{
	std::lock_guard lock(queue_mutex);
	bShouldFinish = true;
	tasks = std::queue<std::function<void()>> {};
	cv.notify_all();
}

void MultiThreadEngine::wait()
{
	std::unique_lock lock(wait_mutex);
	wait_cv.wait(lock, [this]() { 
		std::lock_guard queue_lock(queue_mutex);
		bool all_ready = true;
		for (const auto& thread : threads) {
			if (!thread.is_ready.load()) {
				all_ready = false;
				break;
			}
		}
		return all_ready && tasks.empty();
		});
}

size_t MultiThreadEngine::getTasksCount() const {
	std::lock_guard lock(queue_mutex);
	return tasks.size();
}

size_t MultiThreadEngine::getThreadsCount() const
{
	return threads.size();
}

MultiThreadEngine::MultiThreadEngine(unsigned int threads_count): threads(threads_count)
{
	for (auto& thr : threads)
		thr.thread = std::thread(work, this, std::ref(thr));
}

MultiThreadEngine::~MultiThreadEngine()
{
	bShouldFinish.store(true);
	cv.notify_all();
	for (auto& thread : threads)
		if (thread.thread.joinable())
			thread.thread.join();
}
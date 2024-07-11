#include "MultyThreadEngine.h"

static void work(MultyThreadEngine* mte) {
	while (true) {
		std::function<void()> task;
		{
			std::unique_lock<std::recursive_mutex> lock(mte->queue_mutex);
			mte->cv.wait(lock, [mte]() {
				return mte->shouldStop() || (mte->tasks.empty() && mte->shouldFinish()) || !mte->tasks.empty();
				});

			if (mte->shouldStop() || mte->getTasksCount() == 0ull)
				return;

			task = std::move(mte->tasks.front());
			mte->tasks.pop();
		}
		task();
	}
}
void MultyThreadEngine::addTask(std::function<void()> task) {
	{
		std::lock_guard lock(queue_mutex);
		tasks.push(task);
	}
	cv.notify_one();
}

bool MultyThreadEngine::shouldFinish() const {
	// std::lock_guard<std::mutex> lock(queue_mutex);
	return bShouldFinish.load();
}
bool MultyThreadEngine::shouldStop() const {
	// std::lock_guard<std::mutex> lock(queue_mutex);
	return bShouldStop.load();
}

void MultyThreadEngine::stop()
{
	std::lock_guard lock(queue_mutex);
	bShouldStop = true;
	cv.notify_all();
}

size_t MultyThreadEngine::getTasksCount() const {
	std::lock_guard lock(queue_mutex);
	return tasks.size();
}

MultyThreadEngine::MultyThreadEngine()
{
	const unsigned int thc = std::thread::hardware_concurrency();
	for (auto& thread : threads)
		thread = std::thread(work, this);
}

MultyThreadEngine::MultyThreadEngine(unsigned int threads_count): threads(threads_count)
{
	for (auto& thread : threads)
		thread = std::thread(work, this);
}

MultyThreadEngine::~MultyThreadEngine()
{
	bShouldFinish.store(true);
	cv.notify_all();
	for (auto& thread : threads)
		if (thread.joinable())
			thread.join();
}
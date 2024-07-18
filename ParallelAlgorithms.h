#pragma once
#include <thread>
#include <atomic>
#include <algorithm>
#include <vector>
#include <functional>
#include "Spliter.h"
#include "MultiThreadEngine.h"

#define MIN_SUBARRAY_SIZE 10000llu



namespace pal
{
	// Divide array on parts, sort them and merge sorted parts.
	template<typename RandIt>
	void sort_by_parts(RandIt begin, RandIt end, unsigned int threads_count = 0);
	
	// Divide array on pairs and perform `action` on each pair
	template<typename RandIt, typename PairFunc>
	void make_by_pair(RandIt begin, RandIt end, PairFunc action, unsigned int threads_count = 0);
	
	template<typename GeneratorFunc, typename TestFunc, typename CheckFunc>
	double test_function(GeneratorFunc genFunc, TestFunc testFunc, CheckFunc checkFunc, unsigned int count_of_tests, unsigned int threads_count = 1);
};

template<typename ResultType, typename GeneratorFunc, typename TestFunc, typename CheckFunc>
static void test_perform(ResultType& general_duration, GeneratorFunc genFunc, TestFunc testFunc, CheckFunc checkFunc, unsigned int count_of_tests) {
	for (size_t i = 0; i < count_of_tests; ++i) {
		if constexpr (std::is_same_v<GeneratorFunc, std::nullptr_t> && std::is_same_v<CheckFunc, std::nullptr_t>) {
			const auto start = std::chrono::high_resolution_clock::now();
			testFunc();
			const auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> duration = end - start;
			general_duration += duration.count();
		}
		else if constexpr (!std::is_same_v<GeneratorFunc, std::nullptr_t> && !std::is_same_v<CheckFunc, std::nullptr_t>) {
			auto data = genFunc();
			const auto start = std::chrono::high_resolution_clock::now();
			auto res = testFunc(data);
			const auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> duration = end - start;
			general_duration += duration.count();
			checkFunc(data, res);
		}
		else if constexpr (!std::is_same_v<GeneratorFunc, std::nullptr_t>) {
			auto data = genFunc();
			const auto start = std::chrono::high_resolution_clock::now();
			testFunc(data);
			const auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> duration = end - start;
			general_duration += duration.count();
		}
		else {
			const auto start = std::chrono::high_resolution_clock::now();
			auto res = testFunc();
			const auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> duration = end - start;
			general_duration += duration.count();
			checkFunc(res);
		}
	}
}

template<typename GeneratorFunc, typename TestFunc, typename CheckFunc>
double pal::test_function(GeneratorFunc genFunc, TestFunc testFunc, CheckFunc checkFunc, unsigned int count_of_tests, unsigned int threads_count) {
	if (threads_count == 0)
		threads_count = std::thread::hardware_concurrency();
	threads_count = std::min({ std::thread::hardware_concurrency(), threads_count , count_of_tests });
	const unsigned int thc = threads_count == 0 ? 1 : threads_count;

	if (thc > 1) {
		MultyThreadEngine mte(thc);
		std::atomic<double> general_duration(0.);
		auto task = [&general_duration](GeneratorFunc genFunc, TestFunc testFunc, CheckFunc checkFunc, unsigned int count_of_tests) {
			test_perform(general_duration, genFunc, testFunc, checkFunc, count_of_tests);
		};
		for (size_t i = 0; i < thc-1; ++i) {
			mte.addTask([task, genFunc, testFunc, checkFunc, count_of_tests , thc]() {
				task(genFunc, testFunc, checkFunc, count_of_tests / thc);
				});
		}
		mte.addTask([task, genFunc, testFunc, checkFunc, count_of_tests, thc]() {
			task(genFunc, testFunc, checkFunc, count_of_tests / thc + count_of_tests % thc);
			});
		mte.wait();
		return general_duration.load()/count_of_tests;
	}

	double general_duration = 0.;
	test_perform(general_duration, genFunc, testFunc, checkFunc, count_of_tests);
	return general_duration / count_of_tests;
}

template<typename RandIt, typename PairFunc>
void pal::make_by_pair(RandIt begin, RandIt end, PairFunc action, unsigned int threads_count) {
	if (std::thread::hardware_concurrency() < threads_count || threads_count == 0)
		threads_count = std::thread::hardware_concurrency();
	const size_t len = end - begin;
	const unsigned int max_thc = len / 2llu;
	const unsigned int thc = std::min(threads_count, max_thc);

	const auto performer = [action](RandIt begin, RandIt end) {
		int t = 0;
		for (RandIt it = begin; it < end && it+1 < end; it+=2, t+=2) {
			action(*it, *(it+1));
			}
		};

	if (thc <= 1) {
		performer(begin, end);
		return;
	}
	std::vector<Part<RandIt>> parts = split_to_parts(begin, end, thc);
	std::vector<std::thread> threads(thc);
	
	for (size_t i = 0; i < thc; ++i) {
		threads[i] = std::thread(performer, parts[i].begin, parts[i].end);
	}
	for (auto& thread : threads) {
		thread.join();
	}
}

template<typename RandIt>
void pal::sort_by_parts(RandIt begin, RandIt end, unsigned int threads_count)
{
	if (std::thread::hardware_concurrency() < threads_count || threads_count == 0)
		threads_count = std::thread::hardware_concurrency();
	const size_t len = end - begin;
	const unsigned int recomended_thc = len/MIN_SUBARRAY_SIZE;
	const unsigned int thc = std::min(recomended_thc, threads_count);

	if (thc <= 1) {
		std::sort(begin, end);
		return;
	}

	std::vector<Part<RandIt>> parts = split_to_parts(begin, end, thc);
	std::vector<std::thread> threads(thc);

	for (size_t i = 0; i < thc; ++i) {
		threads[i] = std::thread([&parts, i]() {
			std::sort(parts[i].begin, parts[i].end);
			});
	}
	for (auto& thread : threads) {
		thread.join();
	}

	unsigned int remaining_count = thc;
	while (remaining_count > 1) {
		pal::make_by_pair(parts.begin(), parts.begin() + remaining_count, [](Part<RandIt> l, Part<RandIt> r) {
			std::inplace_merge(l.begin, r.begin, r.end);
			}, threads_count);
		for (unsigned int i = 0; i*2 < remaining_count; ++i) {
			if (i * 2 + 1 < remaining_count)
				parts[i] = parts[i * 2].merge(parts[i * 2 + 1]);
			else
				parts[i] = parts[remaining_count-1];
		}
		remaining_count = ceil(remaining_count/2.);
	}
}
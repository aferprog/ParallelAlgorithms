#pragma once
#include <thread>
#include <atomic>
#include <algorithm>
#include <vector>
#include <functional>

#define MIN_SUBARRAY_SIZE 10000llu

template<typename RandIt>
struct Part {
	RandIt begin, end;
	size_t begin_index, end_index;
	RandIt getMiddleIt() const {
		return begin + (end- begin)/2;
	}
	RandIt getMiddleInd() const {
		return begin + (end - begin) / 2;
	}

	Part<RandIt> merge(Part<RandIt> right) {
		return {
			begin, right.end,
			begin_index, right.end_index
		};
	}
	size_t getSize() {
		return end_index - begin_index;
	}
};

template<typename RandIt>
std::vector<struct Part<RandIt>> split_to_parts(RandIt begin, RandIt end, unsigned int count) {
	const size_t len = end - begin;
	const size_t base_part_len = len / count;

	std::vector<struct Part<RandIt>> res(count);
	unsigned int i;
	for (i = 0; i < count-1; ++i) {
		res[i].begin_index = base_part_len * i;
		res[i].end_index = base_part_len * (i + 1);
		res[i].begin = begin + res[i].begin_index;
		res[i].end = begin + res[i].end_index;
	}
	res[i].begin_index = base_part_len * i;
	res[i].end_index = len;
	res[i].begin = begin + res[i].begin_index;
	res[i].end = end;
	return res;
}

namespace pal
{
	// Divide array on parts, sort them and merge sorted parts.
	template<typename RandIt>
	void sort_by_parts(RandIt begin, RandIt end, unsigned int threads_count = 0);
	
	// Divide array on pairs and perform `action` on each pair
	template<typename RandIt, typename PairFunc>
	void make_by_pair(RandIt begin, RandIt end, PairFunc action, unsigned int threads_count = 0);
	
	/*template<typename TestFunc, typename CheckFunc>
	double test_function(TestFunc testFunc, CheckFunc checkFunc, unsigned int threads_count = 0);*/
};

//template<typename TestFunc, typename CheckFunc>
//double pal::test_function(TestFunc testFunc, CheckFunc checkFunc, unsigned int threads_count) {
//	
//}

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
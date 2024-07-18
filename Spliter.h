#pragma once
#include <vector>

namespace pal {
	template<typename RandIt>
	struct Part {
		RandIt begin, end;
		size_t begin_index, end_index;
		RandIt getMiddleIt() const {
			return begin + (end - begin) / 2;
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
		for (i = 0; i < count - 1; ++i) {
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
}
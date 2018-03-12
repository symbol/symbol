#include "catapult/utils/Functional.h"
#include "tests/TestHarness.h"
#include <algorithm>
#include <list>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

namespace catapult { namespace utils {

#define TEST_CLASS FunctionalTests

	TEST(TEST_CLASS, RunningReduceOnAnEmptyContainerYieldsInitialValue) {
		// Arrange:
		std::vector<int> vec;

		// Act:
		auto result = Reduce(vec, 12345, [](auto, auto) { return 54321; });

		// Assert:
		EXPECT_EQ(result, 12345);
	}

	TEST(TEST_CLASS, RunningReduceOnNonEmptyContainerExecutesCallbackOnAllElements) {
		// Arrange:
		std::vector<int> vec{ 8, 13, 21, 34, 55 };

		// Act:
		std::vector<std::pair<int, int>> collected;
		auto result = Reduce(vec, 12345, [&](auto a, auto b) {
			collected.push_back(std::make_pair(a, b));
			return b;
		});

		// Assert:
		std::vector<std::pair<int, int>> expected{
			{ 12345, 8 },
			{ 8, 13 },
			{ 13, 21 },
			{ 21, 34 },
			{ 34, 55 },
		};
		EXPECT_EQ(result, 55);
		EXPECT_EQ(expected, collected);
	}

	namespace {
		template<typename Container>
		void AssertReduce() {
			// Arrange:
			Container container;
			for (auto i = 0u; i < 10; ++i)
				container.insert(container.end(), test::Random());

			// Act:
			auto largest = std::numeric_limits<typename Container::value_type>::max();
			uint64_t const & (*fun) (uint64_t const &, uint64_t const &) = std::min<uint64_t>;
			auto result = Reduce(container, largest, fun);

			// Assert:
			auto expected = std::accumulate(container.begin(), container.end(), largest, fun);
			EXPECT_EQ(expected, result);
		}
	}

	TEST(TEST_CLASS, CanRunReduceOnDifferentContainerTypes) {
		AssertReduce<std::list<uint64_t>>();
		AssertReduce<std::set<uint64_t>>();
		AssertReduce<std::vector<uint64_t>>();
		AssertReduce<std::unordered_set<uint64_t>>();
	}

	TEST(TEST_CLASS, ReturnedValueIsOfInitType) {
		// Arrange:
		std::map<std::string, int> container{
			{ "alpha", 21 },
			{ "bravo", 22 },
			{ "charlie", 23 },
			{ "delta", 24 },
			{ "echo", 25 }
		};

		// Act:
		size_t len = 0;
		auto result = Reduce(container, len, [](auto a, const auto& b) {
			return 1 == b.second % 2
				? a + b.first.size()
				: a;
		});

		// Assert:
		EXPECT_EQ(5u + 7 + 4, result);
	}

	TEST(TEST_CLASS, SumOnEmptyContainerReturnsZero) {
		// Arrange:
		std::vector<int> data;

		// Act:
		auto result = Sum(data, [](auto value) { return value; });

		// Assert:
		EXPECT_EQ(0, result);
	}

	TEST(TEST_CLASS, SumCallbackIsCalledOnEveryElement) {
		// Arrange:
		std::vector<int> data{ 8, 13, 21, 34, 55 };

		// Act:
		std::vector<int> collected;
		Sum(data, [&collected](auto value) {
			collected.push_back(value);
			return 0;
		});

		// Assert:
		EXPECT_EQ(data, collected);
	}

	TEST(TEST_CLASS, SumReturnsProperValue_SingleElement) {
		// Arrange:
		std::vector<int> data{ 8 };

		// Act:
		auto result = Sum(data, [](auto value) { return value; });

		// Assert:
		EXPECT_EQ(8, result);
	}

	TEST(TEST_CLASS, SumReturnsProperValue_MultipleElements) {
		// Arrange:
		std::vector<int> data{ 8, 13, 21, 34, 55 };

		// Act:
		auto result = Sum(data, [](auto value) { return value; });

		// Assert:
		EXPECT_EQ(131, result);
	}
}}

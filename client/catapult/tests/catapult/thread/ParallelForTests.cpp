/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "catapult/thread/ParallelFor.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/nodeps/BasicMultiThreadedState.h"
#include "tests/TestHarness.h"
#include <list>
#include <numeric>

namespace catapult { namespace thread {

#define TEST_CLASS ParallelForTests

	namespace {
		using ItemType = uint32_t;

		std::vector<ItemType> CreateIncrementingValues(size_t size) {
			auto items = std::vector<ItemType>(size);
			std::iota(items.begin(), items.end(), static_cast<ItemType>(1));
			return items;
		}

		template<typename TContainer>
		struct BasicTestContext {
		public:
			explicit BasicTestContext(size_t numItemsAdjustment = 0)
					: pPool(test::CreateStartedIoThreadPool())
					, NumThreads(pPool->numWorkerThreads())
					, NumItems(NumThreads * 5 + numItemsAdjustment)
					, ItemsSum((NumItems * (NumItems + 1)) / 2) {
				auto seedItems = CreateIncrementingValues(NumItems);
				std::copy(seedItems.cbegin(), seedItems.cend(), std::back_inserter(Items));
			}

		public:
			std::unique_ptr<thread::IoThreadPool> pPool;
			size_t NumThreads;
			size_t NumItems;
			size_t ItemsSum;
			TContainer Items;
		};

		struct VectorTraits {
			using ContainerType = std::vector<ItemType>;
		};

		struct ListTraits {
			using ContainerType = std::list<ItemType>;
		};
	}

#define CONTAINER_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Vector) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<VectorTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_List) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ListTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region ParallelForPartition basic

	CONTAINER_TEST(CanProcessMultiplePartitionsConcurrently_ZeroItems) {
		// Arrange:
		BasicTestContext<typename TTraits::ContainerType> context;
		auto items = typename TTraits::ContainerType();

		// Act:
		std::atomic<size_t> counter(0);
		ParallelForPartition(context.pPool->ioContext(), items, context.NumThreads, [&counter](auto, auto, auto, auto) {
			++counter;
		}).get();

		// Assert: the partition callback was not called
		EXPECT_EQ(0u, counter);
	}

	namespace {
		struct PartitionAggregateCapture {
		public:
			PartitionAggregateCapture(size_t numItems, size_t numPartitions)
					: Sum(0)
					, IndexFlags(numItems, 0)
					, BatchIndexFlags(numPartitions, 0)
			{}

		public:
			std::atomic<size_t> Sum;

			// use vector of uint8_t instead of bool because latter does not guarantee that
			// different elements in the same container can be modified concurrently by different threads
			std::vector<uint8_t> IndexFlags;
			std::vector<uint8_t> BatchIndexFlags;
		};

		auto CreatePartitionAggregate(PartitionAggregateCapture& capture) {
			return [&capture](auto itBegin, auto itEnd, auto startIndex, auto batchIndex) {
				// Sanity: fail if any index is too large
				ASSERT_GT(capture.IndexFlags.size(), startIndex) << "unexpected start index " << startIndex;
				ASSERT_GT(capture.BatchIndexFlags.size(), batchIndex) << "unexpected batch index " << batchIndex;

				// Act:
				++capture.BatchIndexFlags[batchIndex];
				for (auto iter = itBegin; itEnd != iter; ++iter) {
					++capture.IndexFlags[startIndex++]; // use start index to visit all items
					capture.Sum += *iter;
				}
			};
		}
	}

	CONTAINER_TEST(CanProcessMultiplePartitionsConcurrently_OneItem) {
		// Arrange:
		BasicTestContext<typename TTraits::ContainerType> context;
		auto items = typename TTraits::ContainerType{ 7 };

		// Act:
		PartitionAggregateCapture capture(1, 1);
		ParallelForPartition(context.pPool->ioContext(), items, context.NumThreads, CreatePartitionAggregate(capture)).get();

		// Assert: the callback was only called once (since there is only one item and one partition)
		EXPECT_EQ(7u, capture.Sum);
		EXPECT_EQ(std::vector<uint8_t>(1, 1), capture.IndexFlags);
		EXPECT_EQ(std::vector<uint8_t>(1, 1), capture.BatchIndexFlags);
	}

	namespace {
		template<typename TTraits>
		void AssertCanProcessMultiplePartitionsConcurrently(int numItemsAdjustment) {
			// Arrange:
			BasicTestContext<typename TTraits::ContainerType> context(static_cast<size_t>(numItemsAdjustment));

			// Act:
			PartitionAggregateCapture capture(context.Items.size(), context.NumThreads);
			ParallelForPartition(context.pPool->ioContext(), context.Items, context.NumThreads, CreatePartitionAggregate(capture)).get();

			// Assert:
			EXPECT_EQ(context.ItemsSum, capture.Sum);
			EXPECT_EQ(std::vector<uint8_t>(context.Items.size(), 1), capture.IndexFlags);
			EXPECT_EQ(std::vector<uint8_t>(context.NumThreads, 1), capture.BatchIndexFlags);
		}
	}

	CONTAINER_TEST(CanProcessMultiplePartitionsConcurrently_MinusOne) {
		AssertCanProcessMultiplePartitionsConcurrently<TTraits>(-1);
	}

	CONTAINER_TEST(CanProcessMultiplePartitionsConcurrently) {
		AssertCanProcessMultiplePartitionsConcurrently<TTraits>(0);
	}

	CONTAINER_TEST(CanProcessMultiplePartitionsConcurrently_PlusOne) {
		AssertCanProcessMultiplePartitionsConcurrently<TTraits>(1);
	}

	CONTAINER_TEST(CanModifyMultiplePartitionsConcurrently) {
		// Arrange:
		BasicTestContext<typename TTraits::ContainerType> context;

		// Act:
		ParallelForPartition(context.pPool->ioContext(), context.Items, context.NumThreads, [](auto itBegin, auto itEnd, auto, auto) {
			for (auto iter = itBegin; itEnd != iter; ++iter)
				*iter = *iter * *iter + 1;
		}).get();

		// Assert: all values should have been modified
		auto i = 1u;
		for (auto value : context.Items) {
			EXPECT_EQ(i * i + 1u, value) << "item at " << i;
			++i;
		}
	}

	// endregion

	// region ParallelFor basic

	CONTAINER_TEST(CanProcessMultipleItemsConcurrently_ZeroItems) {
		// Arrange:
		BasicTestContext<typename TTraits::ContainerType> context;
		auto items = typename TTraits::ContainerType();

		// Act:
		std::atomic<size_t> counter(0);
		ParallelFor(context.pPool->ioContext(), items, context.NumThreads, [&counter](auto, auto) {
			++counter;
			return true;
		}).get();

		// Assert: the item callback was not called
		EXPECT_EQ(0u, counter);
	}

	// use vector of uint8_t instead of bool because latter does not guarantee that
	// different elements in the same container can be modified concurrently by different threads
	namespace {
		auto CreateItemAggregate(std::atomic<size_t>& sum, std::vector<uint8_t>& indexFlags) {
			return [&sum, &indexFlags](auto value, auto index) {
				// Sanity: fail if any index is too large
				EXPECT_GT(indexFlags.size(), index) << "unexpected index " << index;
				if (indexFlags.size() <= index)
					return false;

				sum += value;
				++indexFlags[index];
				return true;
			};
		}
	}

	CONTAINER_TEST(CanProcessMultipleItemsConcurrently_OneItem) {
		// Arrange:
		BasicTestContext<typename TTraits::ContainerType> context;
		auto items = typename TTraits::ContainerType{ 7 };

		// Act:
		std::atomic<size_t> sum(0);
		std::vector<uint8_t> indexFlags(1, 0);
		ParallelFor(context.pPool->ioContext(), items, context.NumThreads, CreateItemAggregate(sum, indexFlags)).get();

		// Assert: the callback was only called once (since there is only one item)
		EXPECT_EQ(7u, sum);
		EXPECT_EQ(std::vector<uint8_t>(1, 1), indexFlags);
	}

	namespace {
		template<typename TTraits>
		void AssertCanProcessMultipleItemsConcurrently(int numItemsAdjustment) {
			// Arrange:
			BasicTestContext<typename TTraits::ContainerType> context(static_cast<size_t>(numItemsAdjustment));

			// Act:
			std::atomic<size_t> sum(0);
			std::vector<uint8_t> indexFlags(context.NumItems, 0);
			ParallelFor(context.pPool->ioContext(), context.Items, context.NumThreads, CreateItemAggregate(sum, indexFlags)).get();

			// Assert:
			EXPECT_EQ(context.ItemsSum, sum);
			EXPECT_EQ(std::vector<uint8_t>(context.NumItems, 1), indexFlags);
		}
	}

	CONTAINER_TEST(CanProcessMultipleItemsConcurrently_MinusOne) {
		AssertCanProcessMultipleItemsConcurrently<TTraits>(-1);
	}

	CONTAINER_TEST(CanProcessMultipleItemsConcurrently) {
		AssertCanProcessMultipleItemsConcurrently<TTraits>(0);
	}

	CONTAINER_TEST(CanProcessMultipleItemsConcurrently_PlusOne) {
		AssertCanProcessMultipleItemsConcurrently<TTraits>(1);
	}

	CONTAINER_TEST(CanShortCircuitItemProcessing) {
		// Arrange:
		BasicTestContext<typename TTraits::ContainerType> context;

		// Act:
		std::atomic<size_t> sum(0);
		ParallelFor(context.pPool->ioContext(), context.Items, context.NumThreads, [&sum, itemsSum = context.ItemsSum](auto value, auto) {
			sum += value;
			return itemsSum < sum;
		}).get();

		// Assert:
		EXPECT_GT(context.ItemsSum, sum);
	}

	CONTAINER_TEST(CanModifyMultipleItemsConcurrently) {
		// Arrange:
		BasicTestContext<typename TTraits::ContainerType> context;

		// Act:
		ParallelFor(context.pPool->ioContext(), context.Items, context.NumThreads, [](auto& value, auto) {
			value = value * value + 1;
			return true;
		}).get();

		// Assert: all values should have been modified
		auto i = 1u;
		for (auto value : context.Items) {
			EXPECT_EQ(i * i + 1u, value) << "item at " << i;
			++i;
		}
	}

	CONTAINER_TEST(CorrectIndexesAreAssociatedWithItems) {
		// Arrange:
		BasicTestContext<typename TTraits::ContainerType> context;

		// Act: capture all values by their index
		std::vector<uint32_t> capturedValues(context.NumItems, 0);
		ParallelFor(context.pPool->ioContext(), context.Items, context.NumThreads, [&capturedValues](auto value, auto index) {
			// Sanity: fail if any index is too large
			EXPECT_GT(capturedValues.size(), index) << "unexpected index " << index;
			if (capturedValues.size() <= index)
				return false;

			capturedValues[index] = value;
			return true;
		}).get();

		// Assert: values start at 1
		for (auto i = 0u; i < capturedValues.size(); ++i)
			EXPECT_EQ(i + 1, capturedValues[i]) << "i " << i;
	}

	// endregion

	// region ParallelFor[Partition] distributed

	namespace {
		struct ParallelForTraits {
			using ItemType = decltype(CreateIncrementingValues(0))::value_type;

			static uint64_t GetValue(ItemType item) {
				return item;
			}
		};

		using MultiThreadedState = test::BasicMultiThreadedState<ParallelForTraits>;

		struct DistributeParallelForTraits {
			static void ParallelFor(
					boost::asio::io_context& ioContext,
					const std::vector<ItemType>& items,
					size_t numThreads,
					MultiThreadedState& state) {
				std::atomic<size_t> numItemsProcessed(0);
				thread::ParallelFor(ioContext, items, numThreads, [&state, &numItemsProcessed, numThreads](auto value, auto) {
					// - process the value
					state.process(value);

					// - wait for all threads to spawn before continuing
					++numItemsProcessed;
					WAIT_FOR_EXPR(numItemsProcessed >= numThreads);
					return true;
				}).get();
			}
		};

		struct DistributeParallelForPartitionTraits {
			static void ParallelFor(
					boost::asio::io_context& ioContext,
					const std::vector<ItemType>& items,
					size_t numThreads,
					MultiThreadedState& state) {
				std::atomic<size_t> numItemsProcessed(0);
				ParallelForPartition(ioContext, items, numThreads, [&state, &numItemsProcessed, numThreads](
						auto itBegin,
						auto itEnd,
						auto,
						auto) {
					// - process the values
					for (auto iter = itBegin; itEnd != iter; ++iter)
						state.process(*iter);

					// - wait for all threads to spawn before continuing
					++numItemsProcessed;
					WAIT_FOR_EXPR(numItemsProcessed >= numThreads);
					return true;
				}).get();
			}
		};

		template<typename TParallelFunc>
		void AssertCanDistributeWorkEvenly(size_t multiplier, size_t divisor, TParallelFunc parallelFunc) {
			// Arrange:
			auto pPool = test::CreateStartedIoThreadPool();
			auto numThreads = pPool->numWorkerThreads();
			auto numItems = numThreads * multiplier / divisor;
			auto items = CreateIncrementingValues(numItems);

			// Sanity: Num_Default_Threads is 2 * cores, so Num_Default_Threads / 4 is nonzero when there are at least two cores
			ASSERT_NE(0u, numItems) << "test is not supported on single core system";

			// Act:
			MultiThreadedState state;
			parallelFunc(pPool->ioContext(), items, numThreads, state);

			// Assert: all items were processed once
			EXPECT_EQ(numItems, state.counter());
			EXPECT_EQ(numItems, state.numUniqueItems());

			// - multiple execution threads were used
			EXPECT_EQ(numThreads, state.threadCounters().size());
			EXPECT_EQ(numThreads, state.sortedAndReducedThreadIds().size());

			// - the work was distributed evenly across threads
			//   (a thread can do more than the min amount of work if the number of items is not divisible by the number of threads)
			auto minWorkPerThread = numItems / numThreads;
			for (auto counter : state.threadCounters())
				EXPECT_LE(minWorkPerThread, counter);
		}

#define DISTRIBUTE_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<DistributeParallelForTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Partition) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<DistributeParallelForPartitionTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()
	}

	DISTRIBUTE_TEST(CanDistributeWorkEvenlyWhenItemsAreMultipleOfThreads) {
		AssertCanDistributeWorkEvenly(20, 1, TTraits::ParallelFor);
	}

	DISTRIBUTE_TEST(CanDistributeWorkEvenlyWhenItemsAreNotMultipleOfThreads) {
		AssertCanDistributeWorkEvenly(81, 4, TTraits::ParallelFor);
	}

	// endregion
}}

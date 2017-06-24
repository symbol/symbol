#include "catapult/thread/ParallelFor.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/nodeps/BasicMultiThreadedState.h"
#include "tests/TestHarness.h"
#include <list>
#include <numeric>

#define TEST_CLASS ParallelForTests

namespace catapult { namespace thread {

	namespace {
		using ItemType = uint16_t;

		std::vector<ItemType> CreateIncrementingValues(size_t size) {
			auto items = std::vector<ItemType>(size);
			std::iota(items.begin(), items.end(), 1);
			return items;
		}

		template<typename TContainer>
		struct BasicTestContext {
		public:
			explicit BasicTestContext(size_t numItemsAdjustment = 0)
					: pPool(test::CreateStartedIoServiceThreadPool())
					, NumThreads(pPool->numWorkerThreads())
					, NumItems(NumThreads * 5 + numItemsAdjustment)
					, ItemsSum((NumItems * (NumItems + 1)) / 2) {
				auto seedItems = CreateIncrementingValues(NumItems);
				std::copy(seedItems.cbegin(), seedItems.cend(), std::back_inserter(Items));
			}

		public:
			std::unique_ptr<thread::IoServiceThreadPool> pPool;
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

#define CONTAINER_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Vector) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<VectorTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_List) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ListTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()
	}

	// region ParallelForPartition basic

	CONTAINER_TEST(CanProcessMultiplePartitionsConcurrently_ZeroItems) {
		// Arrange:
		BasicTestContext<typename TTraits::ContainerType> context;
		auto items = typename TTraits::ContainerType();

		// Act:
		std::atomic<size_t> counter(0);
		ParallelForPartition(context.pPool->service(), items, context.NumThreads, [&counter](auto, auto, auto) {
			++counter;
		}).get();

		// Assert: the partition callback was not called
		EXPECT_EQ(0u, counter);
	}

	namespace {
		// use vector of uint8_t instead of bool because latter does not guarantee that
		// different elements in the same container can be modified concurrently by different threads
		auto CreatePartitionAggregate(std::atomic<size_t>& sum, std::vector<uint8_t>& indexFlags) {
			return [&sum, &indexFlags](auto itBegin, auto itEnd, auto index) {
				// Sanity: fail if any index is too large
				ASSERT_GT(indexFlags.size(), index) << "unexpected index " << index;

				// Act:
				indexFlags[index] = 1;
				for (auto iter = itBegin; itEnd != iter; ++iter)
					sum += *iter;
			};
		}
	}

	CONTAINER_TEST(CanProcessMultiplePartitionsConcurrently_OneItem) {
		// Arrange:
		BasicTestContext<typename TTraits::ContainerType> context;
		auto items = typename TTraits::ContainerType{ 7 };

		// Act:
		std::atomic<size_t> sum(0);
		std::vector<uint8_t> indexFlags(1, 0);
		ParallelForPartition(context.pPool->service(), items, context.NumThreads, CreatePartitionAggregate(sum, indexFlags)).get();

		// Assert: the callback was only called once (since there is only one item and one partition)
		EXPECT_EQ(7u, sum);
		EXPECT_EQ(std::vector<uint8_t>(1, 1), indexFlags);
	}

	namespace {
		template<typename TTraits>
		void AssertCanProcessMultiplePartitionsConcurrently(int numItemsAdjustment) {
			// Arrange:
			BasicTestContext<typename TTraits::ContainerType> context(static_cast<size_t>(numItemsAdjustment));

			// Act:
			std::atomic<size_t> sum(0);
			std::vector<uint8_t> indexFlags(context.NumThreads, 0);
			auto partitionAggregate = CreatePartitionAggregate(sum, indexFlags);
			ParallelForPartition(context.pPool->service(), context.Items, context.NumThreads, partitionAggregate).get();

			// Assert:
			EXPECT_EQ(context.ItemsSum, sum);
			EXPECT_EQ(std::vector<uint8_t>(context.NumThreads, 1), indexFlags);
		}
	}

	CONTAINER_TEST(CanProcessMultiplePartitionsConcurrently_MinusOne) {
		// Assert:
		AssertCanProcessMultiplePartitionsConcurrently<TTraits>(-1);
	}

	CONTAINER_TEST(CanProcessMultiplePartitionsConcurrently) {
		// Assert:
		AssertCanProcessMultiplePartitionsConcurrently<TTraits>(0);
	}

	CONTAINER_TEST(CanProcessMultiplePartitionsConcurrently_PlusOne) {
		// Assert:
		AssertCanProcessMultiplePartitionsConcurrently<TTraits>(1);
	}

	CONTAINER_TEST(CanModifyMultiplePartitionsConcurrently) {
		// Arrange:
		BasicTestContext<typename TTraits::ContainerType> context;

		// Act:
		ParallelForPartition(context.pPool->service(), context.Items, context.NumThreads, [](auto itBegin, auto itEnd, auto) {
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
		ParallelFor(context.pPool->service(), items, context.NumThreads, [&counter](auto) {
			++counter;
			return true;
		}).get();

		// Assert: the item callback was not called
		EXPECT_EQ(0u, counter);
	}

	namespace {
		auto CreateItemAggregate(std::atomic<size_t>& sum) {
			return [&sum](auto value) {
				sum += value;
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
		ParallelFor(context.pPool->service(), items, context.NumThreads, CreateItemAggregate(sum)).get();

		// Assert: the callback was only called once (since there is only one item)
		EXPECT_EQ(7u, sum);
	}

	namespace {
		template<typename TTraits>
		void AssertCanProcessMultipleItemsConcurrently(int numItemsAdjustment) {
			// Arrange:
			BasicTestContext<typename TTraits::ContainerType> context(static_cast<size_t>(numItemsAdjustment));

			// Act:
			std::atomic<size_t> sum(0);
			ParallelFor(context.pPool->service(), context.Items, context.NumThreads, CreateItemAggregate(sum)).get();

			// Assert:
			EXPECT_EQ(context.ItemsSum, sum);
		}
	}

	CONTAINER_TEST(CanProcessMultipleItemsConcurrently_MinusOne) {
		// Assert:
		AssertCanProcessMultipleItemsConcurrently<TTraits>(-1);
	}

	CONTAINER_TEST(CanProcessMultipleItemsConcurrently) {
		// Assert:
		AssertCanProcessMultipleItemsConcurrently<TTraits>(0);
	}

	CONTAINER_TEST(CanProcessMultipleItemsConcurrently_PlusOne) {
		// Assert:
		AssertCanProcessMultipleItemsConcurrently<TTraits>(1);
	}

	CONTAINER_TEST(CanShortCircuitItemProcessing) {
		// Arrange:
		BasicTestContext<typename TTraits::ContainerType> context;

		// Act:
		std::atomic<size_t> sum(0);
		ParallelFor(context.pPool->service(), context.Items, context.NumThreads, [&sum, itemsSum = context.ItemsSum](auto value) {
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
		ParallelFor(context.pPool->service(), context.Items, context.NumThreads, [](auto& value) {
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
					boost::asio::io_service& service,
					const std::vector<ItemType>& items,
					size_t numThreads,
					MultiThreadedState& state) {
				std::atomic<size_t> numItemsProcessed(0);
				thread::ParallelFor(service, items, numThreads, [&state, &numItemsProcessed, numThreads](auto value) {
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
					boost::asio::io_service& service,
					const std::vector<ItemType>& items,
					size_t numThreads,
					MultiThreadedState& state) {
				std::atomic<size_t> numItemsProcessed(0);
				ParallelForPartition(service, items, numThreads, [&state, &numItemsProcessed, numThreads](auto itBegin, auto itEnd, auto) {
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
			auto pPool = test::CreateStartedIoServiceThreadPool();
			auto numThreads = pPool->numWorkerThreads();
			auto numItems = numThreads * multiplier / divisor;
			auto items = CreateIncrementingValues(numItems);

			// Sanity: Num_Default_Threads is 2 * cores, so Num_Default_Threads / 4 is non zero when there are at least two cores
			ASSERT_NE(0u, numItems) << "test is not supported on single core system";

			// Act:
			MultiThreadedState state;
			parallelFunc(pPool->service(), items, numThreads, state);

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
		// Assert:
		AssertCanDistributeWorkEvenly(20, 1, TTraits::ParallelFor);
	}

	DISTRIBUTE_TEST(CanDistributeWorkEvenlyWhenItemsAreNotMultipleOfThreads) {
		// Assert:
		AssertCanDistributeWorkEvenly(81, 4, TTraits::ParallelFor);
	}

	// endregion
}}

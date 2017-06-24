#include "catapult/disruptor/BatchRangeDispatcher.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace disruptor {

	namespace {
		using SourceToRangeMap = std::map<InputSource, model::BlockRange>;

		template<typename TTestFunc>
		void RunTestWithConsumerDispatcher(TTestFunc test) {
			// Arrange:
			SourceToRangeMap inputs;
			auto inputCaptureConsumer = [&inputs](auto& input) {
				// Sanity:
				auto rangeIter = inputs.find(input.source());
				EXPECT_EQ(inputs.cend(), rangeIter) << "unexpected entry for " << input.source();

				inputs.emplace(input.source(), input.detachBlockRange());
				return ConsumerResult::Continue();
			};

			ConsumerDispatcher dispatcher({ "BatchDispatcherTests", 16u }, { inputCaptureConsumer });

			// Act:
			test(dispatcher, inputs);
		}
	}

	// region empty / queue

	TEST(BatchDispatcherTests, BatchDispatcherIsInitiallyEmpty) {
		// Arrange:
		RunTestWithConsumerDispatcher([](auto& dispatcher, const auto&) {
			// Act:
			BatchRangeDispatcher<model::BlockRange> batchDispatcher(dispatcher);

			// Assert:
			EXPECT_TRUE(batchDispatcher.empty());
		});
	}

	TEST(BatchDispatcherTests, CanQueueSingleRange) {
		// Arrange:
		RunTestWithConsumerDispatcher([](auto& dispatcher, const auto&) {
			BatchRangeDispatcher<model::BlockRange> batchDispatcher(dispatcher);

			// Act:
			batchDispatcher.queue(test::CreateBlockEntityRange(7), InputSource::Local);

			// Assert:
			EXPECT_FALSE(batchDispatcher.empty());
		});
	}

	// endregion

	// region dispatch

	TEST(BatchDispatcherTests, DispatchWhenEmptyHasNoEffect) {
		// Arrange:
		RunTestWithConsumerDispatcher([](auto& dispatcher, const auto& inputs) {
			BatchRangeDispatcher<model::BlockRange> batchDispatcher(dispatcher);

			// Act:
			batchDispatcher.dispatch();

			// Assert:
			EXPECT_TRUE(batchDispatcher.empty());
			EXPECT_EQ(0u, dispatcher.numAddedElements());
			EXPECT_EQ(0u, inputs.size());
		});
	}

	namespace {
		model::BlockRange CreateBlockEntityRange(size_t numBlocks, Height height) {
			auto range = test::CreateBlockEntityRange(numBlocks);

			auto i = 0u;
			for (auto& block : range)
				block.Height = height + Height(i++);

			return range;
		}

		void AssertNumForwardedInputs(
				const ConsumerDispatcher& dispatcher,
				const BatchRangeDispatcher<model::BlockRange>& batchDispatcher,
				const SourceToRangeMap& inputs,
				size_t numExpectedInputs) {
			// Assert:
			EXPECT_TRUE(batchDispatcher.empty());
			EXPECT_EQ(numExpectedInputs, dispatcher.numAddedElements());

			// - wait for processing to finish
			WAIT_FOR_VALUE_EXPR(inputs.size(), numExpectedInputs);
			EXPECT_EQ(numExpectedInputs, inputs.size());
		}

		void AssertDispatchedInput(
				const SourceToRangeMap::value_type& entry,
				InputSource expectedSource,
				const std::vector<Height::ValueType>& expectedHeights) {
			// Arrange:
			std::vector<Height::ValueType> heights;
			for (const auto& block : entry.second)
				heights.push_back(block.Height.unwrap());

			// Assert:
			EXPECT_EQ(expectedSource, entry.first);
			EXPECT_EQ(expectedHeights, heights);
		}
	}

	TEST(BatchDispatcherTests, DispatchCanForwardSingleQueuedRange) {
		// Arrange:
		RunTestWithConsumerDispatcher([](auto& dispatcher, const auto& inputs) {
			BatchRangeDispatcher<model::BlockRange> batchDispatcher(dispatcher);
			batchDispatcher.queue(CreateBlockEntityRange(3, Height(6)), InputSource::Local);

			// Act:
			batchDispatcher.dispatch();

			// Assert:
			AssertNumForwardedInputs(dispatcher, batchDispatcher, inputs, 1);

			auto iter = inputs.cbegin();
			AssertDispatchedInput(*iter, InputSource::Local, { 6, 7, 8 });
		});
	}

	TEST(BatchDispatcherTests, DispatchCanForwardMultipleQueuedRangesFromSameSource) {
		// Arrange:
		RunTestWithConsumerDispatcher([](auto& dispatcher, const auto& inputs) {
			BatchRangeDispatcher<model::BlockRange> batchDispatcher(dispatcher);
			batchDispatcher.queue(CreateBlockEntityRange(3, Height(6)), InputSource::Local);
			batchDispatcher.queue(CreateBlockEntityRange(2, Height(10)), InputSource::Local);
			batchDispatcher.queue(CreateBlockEntityRange(4, Height(7)), InputSource::Local);

			// Act:
			batchDispatcher.dispatch();

			// Assert:
			AssertNumForwardedInputs(dispatcher, batchDispatcher, inputs, 1);

			auto iter = inputs.cbegin();
			AssertDispatchedInput(*iter, InputSource::Local, { 6, 7, 8, 10, 11, 7, 8, 9, 10 });
		});
	}

	TEST(BatchDispatcherTests, DispatchCanForwardMultipleQueuedRangesFromDifferentSources) {
		// Arrange:
		RunTestWithConsumerDispatcher([](auto& dispatcher, const auto& inputs) {
			BatchRangeDispatcher<model::BlockRange> batchDispatcher(dispatcher);
			batchDispatcher.queue(CreateBlockEntityRange(3, Height(6)), InputSource::Local);
			batchDispatcher.queue(CreateBlockEntityRange(2, Height(10)), InputSource::Remote_Push);
			batchDispatcher.queue(CreateBlockEntityRange(4, Height(7)), InputSource::Local);
			batchDispatcher.queue(CreateBlockEntityRange(1, Height(50)), InputSource::Remote_Pull);

			// Act:
			batchDispatcher.dispatch();

			// Assert:
			AssertNumForwardedInputs(dispatcher, batchDispatcher, inputs, 3);

			auto iter = inputs.cbegin();
			AssertDispatchedInput(*(iter++), InputSource::Local, { 6, 7, 8, 7, 8, 9, 10 });
			AssertDispatchedInput(*(iter++), InputSource::Remote_Pull, { 50 });
			AssertDispatchedInput(*iter, InputSource::Remote_Push, { 10, 11 });
		});
	}

	// endregion
}}

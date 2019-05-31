/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#include "catapult/disruptor/BatchRangeDispatcher.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace disruptor {

#define TEST_CLASS BatchRangeDispatcherTests

	namespace {
		using SourceToRangeMap = std::map<std::pair<InputSource, Key>, model::BlockRange>;
		using BatchBlockRangeDispatcher = BatchRangeDispatcher<model::AnnotatedBlockRange>;

		template<typename TTestFunc>
		void RunTestWithConsumerDispatcher(TTestFunc test) {
			// Arrange:
			SourceToRangeMap inputs;
			auto inputCaptureConsumer = [&inputs](auto& input) {
				// Sanity:
				auto key = std::make_pair(input.source(), input.sourcePublicKey());
				auto iter = inputs.find(key);
				EXPECT_EQ(inputs.cend(), iter) << "unexpected entry for " << key.first << ", " << key.second;

				inputs.emplace(key, input.detachBlockRange());
				return ConsumerResult::Continue();
			};

			ConsumerDispatcher dispatcher({ "BatchDispatcherTests", 16u }, { inputCaptureConsumer });

			// Act:
			test(dispatcher, inputs);
		}
	}

	// region empty / queue

	TEST(TEST_CLASS, BatchDispatcherIsInitiallyEmpty) {
		// Arrange:
		RunTestWithConsumerDispatcher([](auto& dispatcher, const auto&) {
			// Act:
			BatchBlockRangeDispatcher batchDispatcher(dispatcher);

			// Assert:
			EXPECT_TRUE(batchDispatcher.empty());
		});
	}

	TEST(TEST_CLASS, CanQueueSingleRange) {
		// Arrange:
		RunTestWithConsumerDispatcher([](auto& dispatcher, const auto&) {
			BatchBlockRangeDispatcher batchDispatcher(dispatcher);

			// Act:
			batchDispatcher.queue(test::CreateBlockEntityRange(7), InputSource::Local);

			// Assert:
			EXPECT_FALSE(batchDispatcher.empty());
		});
	}

	// endregion

	// region dispatch

	TEST(TEST_CLASS, DispatchWhenEmptyHasNoEffect) {
		// Arrange:
		RunTestWithConsumerDispatcher([](auto& dispatcher, const auto& inputs) {
			BatchBlockRangeDispatcher batchDispatcher(dispatcher);

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
				const BatchBlockRangeDispatcher& batchDispatcher,
				const SourceToRangeMap& inputs,
				size_t numExpectedInputs) {
			// Assert:
			EXPECT_TRUE(batchDispatcher.empty());
			EXPECT_EQ(numExpectedInputs, dispatcher.numAddedElements());

			// - wait for processing to finish
			WAIT_FOR_VALUE_EXPR(numExpectedInputs, inputs.size());
			EXPECT_EQ(numExpectedInputs, inputs.size());
		}

		void AssertDispatchedInput(
				const SourceToRangeMap& inputs,
				InputSource expectedSource,
				const std::vector<Height::ValueType>& expectedHeights,
				const Key& expectedSourcePublicKey = Key()) {
			// Arrange:
			auto iter = inputs.find(std::make_pair(expectedSource, expectedSourcePublicKey));
			ASSERT_NE(inputs.cend(), iter) << "no entry for " << expectedSource << ", " << expectedSourcePublicKey;
			const auto& entry = *iter;

			std::vector<Height::ValueType> heights;
			for (const auto& block : entry.second)
				heights.push_back(block.Height.unwrap());

			// Assert:
			EXPECT_EQ(expectedSource, entry.first.first);
			EXPECT_EQ(expectedSourcePublicKey, entry.first.second);
			EXPECT_EQ(expectedHeights, heights);
		}
	}

	TEST(TEST_CLASS, DispatchCanForwardSingleQueuedRange) {
		// Arrange:
		RunTestWithConsumerDispatcher([](auto& dispatcher, const auto& inputs) {
			BatchBlockRangeDispatcher batchDispatcher(dispatcher);
			batchDispatcher.queue(CreateBlockEntityRange(3, Height(6)), InputSource::Local);

			// Act:
			batchDispatcher.dispatch();

			// Assert:
			AssertNumForwardedInputs(dispatcher, batchDispatcher, inputs, 1);

			AssertDispatchedInput(inputs, InputSource::Local, { 6, 7, 8 });
		});
	}

	TEST(TEST_CLASS, DispatchCanForwardMultipleQueuedRangesFromSameSource) {
		// Arrange:
		RunTestWithConsumerDispatcher([](auto& dispatcher, const auto& inputs) {
			BatchBlockRangeDispatcher batchDispatcher(dispatcher);
			batchDispatcher.queue(CreateBlockEntityRange(3, Height(6)), InputSource::Local);
			batchDispatcher.queue(CreateBlockEntityRange(2, Height(10)), InputSource::Local);
			batchDispatcher.queue(CreateBlockEntityRange(4, Height(7)), InputSource::Local);

			// Act:
			batchDispatcher.dispatch();

			// Assert:
			AssertNumForwardedInputs(dispatcher, batchDispatcher, inputs, 1);

			AssertDispatchedInput(inputs, InputSource::Local, { 6, 7, 8, 10, 11, 7, 8, 9, 10 });
		});
	}

	TEST(TEST_CLASS, DispatchCanForwardMultipleQueuedRangesFromDifferentSourcesWithSamePublicKey) {
		// Arrange:
		RunTestWithConsumerDispatcher([](auto& dispatcher, const auto& inputs) {
			BatchBlockRangeDispatcher batchDispatcher(dispatcher);
			batchDispatcher.queue(CreateBlockEntityRange(3, Height(6)), InputSource::Local);
			batchDispatcher.queue(CreateBlockEntityRange(2, Height(10)), InputSource::Remote_Push);
			batchDispatcher.queue(CreateBlockEntityRange(4, Height(7)), InputSource::Local);
			batchDispatcher.queue(CreateBlockEntityRange(1, Height(50)), InputSource::Remote_Pull);

			// Act:
			batchDispatcher.dispatch();

			// Assert:
			AssertNumForwardedInputs(dispatcher, batchDispatcher, inputs, 3);

			AssertDispatchedInput(inputs, InputSource::Local, { 6, 7, 8, 7, 8, 9, 10 });
			AssertDispatchedInput(inputs, InputSource::Remote_Pull, { 50 });
			AssertDispatchedInput(inputs, InputSource::Remote_Push, { 10, 11 });
		});
	}

	TEST(TEST_CLASS, DispatchCanForwardMultipleQueuedRangesFromDifferentSourcesAndDifferentPublicKeys) {
		// Arrange:
		RunTestWithConsumerDispatcher([](auto& dispatcher, const auto& inputs) {
			auto keys = test::GenerateRandomDataVector<Key>(3);

			BatchBlockRangeDispatcher batchDispatcher(dispatcher);
			batchDispatcher.queue({ CreateBlockEntityRange(3, Height(6)), keys[1] }, InputSource::Local);
			batchDispatcher.queue({ CreateBlockEntityRange(2, Height(10)), keys[2] }, InputSource::Remote_Push);
			batchDispatcher.queue({ CreateBlockEntityRange(4, Height(7)), keys[1] }, InputSource::Local);
			batchDispatcher.queue({ CreateBlockEntityRange(1, Height(50)), keys[1] }, InputSource::Remote_Pull);
			batchDispatcher.queue({ CreateBlockEntityRange(2, Height(70)), keys[0] }, InputSource::Remote_Push);
			batchDispatcher.queue({ CreateBlockEntityRange(2, Height(60)), keys[2] }, InputSource::Remote_Push);

			// Act:
			batchDispatcher.dispatch();

			// Assert:
			AssertNumForwardedInputs(dispatcher, batchDispatcher, inputs, 4);

			AssertDispatchedInput(inputs, InputSource::Local, { 6, 7, 8, 7, 8, 9, 10 }, keys[1]);
			AssertDispatchedInput(inputs, InputSource::Remote_Push, { 10, 11, 60, 61 }, keys[2]);
			AssertDispatchedInput(inputs, InputSource::Remote_Pull, { 50 }, keys[1]);
			AssertDispatchedInput(inputs, InputSource::Remote_Push, { 70, 71 }, keys[0]);
		});
	}

	// endregion
}}

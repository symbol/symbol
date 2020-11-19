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

#include "catapult/disruptor/BatchRangeDispatcher.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace disruptor {

#define TEST_CLASS BatchRangeDispatcherTests

	namespace {
		constexpr auto Default_Equality_Strategy = model::NodeIdentityEqualityStrategy::Key_And_Host;

		using BatchBlockRangeDispatcher = BatchRangeDispatcher<model::AnnotatedBlockRange>;

		// region SourceToRangeMap

		class SourceToRangeMap {
		public:
			using MapType = std::map<std::tuple<InputSource, Key, std::string>, model::BlockRange>;

		public:
			SourceToRangeMap() : m_size(0)
			{}

		public:
			size_t size() const {
				return m_size;
			}

			const auto& get() const {
				return m_map;
			}

		public:
			void insert(ConsumerInput&& input) {
				// Sanity:
				auto key = std::make_tuple(input.source(), input.sourceIdentity().PublicKey, input.sourceIdentity().Host);
				auto iter = m_map.find(key);
				EXPECT_EQ(m_map.cend(), iter) << "unexpected entry for " << input.source() << ", " << input.sourceIdentity();

				// Act:
				m_map.emplace(key, input.detachBlockRange());
				++m_size;
			}

		private:
			std::atomic<size_t> m_size;
			MapType m_map;
		};

		// endregion

		template<typename TTestFunc>
		void RunTestWithConsumerDispatcher(TTestFunc test) {
			// Arrange:
			SourceToRangeMap inputs;
			auto inputCaptureConsumer = [&inputs](auto&& input) {
				inputs.insert(std::move(input));
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
			BatchBlockRangeDispatcher batchDispatcher(dispatcher, Default_Equality_Strategy);

			// Assert:
			EXPECT_TRUE(batchDispatcher.empty());
		});
	}

	TEST(TEST_CLASS, CanQueueSingleRange) {
		// Arrange:
		RunTestWithConsumerDispatcher([](auto& dispatcher, const auto&) {
			BatchBlockRangeDispatcher batchDispatcher(dispatcher, Default_Equality_Strategy);

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
			BatchBlockRangeDispatcher batchDispatcher(dispatcher, Default_Equality_Strategy);

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
				const Key& expectedSourcePublicKey = Key(),
				const std::string& expectedSourceHost = std::string()) {
			// Arrange:
			auto iter = inputs.get().find(std::make_tuple(expectedSource, expectedSourcePublicKey, expectedSourceHost));
			ASSERT_NE(inputs.get().cend(), iter)
					<< "no entry for (source = " << expectedSource
					<< ", publicKey = " << expectedSourcePublicKey
					<< ", host = " << expectedSourceHost << ")";
			const auto& entry = *iter;

			std::vector<Height::ValueType> heights;
			for (const auto& block : entry.second)
				heights.push_back(block.Height.unwrap());

			// Assert:
			EXPECT_EQ(expectedSource, std::get<0>(entry.first));
			EXPECT_EQ(expectedSourcePublicKey, std::get<1>(entry.first));
			EXPECT_EQ(expectedSourceHost, std::get<2>(entry.first));
			EXPECT_EQ(expectedHeights, heights);
		}
	}

	TEST(TEST_CLASS, DispatchCanForwardSingleQueuedRange) {
		// Arrange:
		RunTestWithConsumerDispatcher([](auto& dispatcher, const auto& inputs) {
			BatchBlockRangeDispatcher batchDispatcher(dispatcher, Default_Equality_Strategy);
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
			BatchBlockRangeDispatcher batchDispatcher(dispatcher, Default_Equality_Strategy);
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

	TEST(TEST_CLASS, DispatchCanForwardMultipleQueuedRangesFromDifferentSourcesWithSameIdentity) {
		// Arrange:
		RunTestWithConsumerDispatcher([](auto& dispatcher, const auto& inputs) {
			BatchBlockRangeDispatcher batchDispatcher(dispatcher, Default_Equality_Strategy);
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

	namespace {
		std::vector<Key> QueueVariedBlockRanges(BatchBlockRangeDispatcher& batchDispatcher) {
			auto keys = test::GenerateRandomDataVector<Key>(3);
			batchDispatcher.queue({ CreateBlockEntityRange(3, Height(6)), { keys[1], "a" } }, InputSource::Local);
			batchDispatcher.queue({ CreateBlockEntityRange(2, Height(10)), { keys[2], "a" } }, InputSource::Remote_Push);
			batchDispatcher.queue({ CreateBlockEntityRange(4, Height(7)), { keys[1], "a" } }, InputSource::Local);
			batchDispatcher.queue({ CreateBlockEntityRange(1, Height(50)), { keys[1], "a" } }, InputSource::Remote_Pull);
			batchDispatcher.queue({ CreateBlockEntityRange(2, Height(70)), { keys[0], "a" } }, InputSource::Remote_Push);
			batchDispatcher.queue({ CreateBlockEntityRange(3, Height(20)), { keys[2], "b" } }, InputSource::Remote_Push);
			batchDispatcher.queue({ CreateBlockEntityRange(2, Height(60)), { keys[2], "a" } }, InputSource::Remote_Push);
			return keys;
		}
	}

	TEST(TEST_CLASS, DispatchCanForwardMultipleQueuedRangesFromDifferentSourcesAndDifferentIdentities) {
		// Arrange:
		RunTestWithConsumerDispatcher([](auto& dispatcher, const auto& inputs) {
			BatchBlockRangeDispatcher batchDispatcher(dispatcher, Default_Equality_Strategy);
			auto keys = QueueVariedBlockRanges(batchDispatcher);

			// Act:
			batchDispatcher.dispatch();

			// Assert:
			AssertNumForwardedInputs(dispatcher, batchDispatcher, inputs, 5);

			AssertDispatchedInput(inputs, InputSource::Local, { 6, 7, 8, 7, 8, 9, 10 }, keys[1], "a");
			AssertDispatchedInput(inputs, InputSource::Remote_Push, { 10, 11, 60, 61 }, keys[2], "a");
			AssertDispatchedInput(inputs, InputSource::Remote_Pull, { 50 }, keys[1], "a");
			AssertDispatchedInput(inputs, InputSource::Remote_Push, { 70, 71 }, keys[0], "a");
			AssertDispatchedInput(inputs, InputSource::Remote_Push, { 20, 21, 22 }, keys[2], "b");
		});
	}

	TEST(TEST_CLASS, DispatchCanForwardMultipleQueuedRangesFromDifferentSourcesAndDifferentIdentities_CustomEqualityStrategy) {
		// Arrange:
		RunTestWithConsumerDispatcher([](auto& dispatcher, const auto& inputs) {
			BatchBlockRangeDispatcher batchDispatcher(dispatcher, model::NodeIdentityEqualityStrategy::Key);
			auto keys = QueueVariedBlockRanges(batchDispatcher);

			// Act:
			batchDispatcher.dispatch();

			// Assert:
			AssertNumForwardedInputs(dispatcher, batchDispatcher, inputs, 4);

			AssertDispatchedInput(inputs, InputSource::Local, { 6, 7, 8, 7, 8, 9, 10 }, keys[1], "a");
			AssertDispatchedInput(inputs, InputSource::Remote_Push, { 10, 11, 20, 21, 22, 60, 61 }, keys[2], "a");
			AssertDispatchedInput(inputs, InputSource::Remote_Pull, { 50 }, keys[1], "a");
			AssertDispatchedInput(inputs, InputSource::Remote_Push, { 70, 71 }, keys[0], "a");
		});
	}

	// endregion
}}

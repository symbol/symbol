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

#include "catapult/consumers/BlockConsumers.h"
#include "tests/catapult/consumers/test/ConsumerInputFactory.h"
#include "tests/catapult/consumers/test/ConsumerTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/nodeps/ParamsCapture.h"
#include "tests/TestHarness.h"

using catapult::disruptor::ConsumerInput;
using catapult::disruptor::InputSource;

namespace catapult { namespace consumers {

#define TEST_CLASS NewBlockConsumerTests

	namespace {
		struct NewBlockSinkParams {
		public:
			explicit NewBlockSinkParams(const std::shared_ptr<const model::Block>& pBlock) : pNewBlock(pBlock)
			{}

		public:
			std::shared_ptr<const model::Block> pNewBlock;
		};

		class MockNewBlockSink : public test::ParamsCapture<NewBlockSinkParams> {
		public:
			void operator()(const std::shared_ptr<const model::Block>& pBlock) const {
				const_cast<MockNewBlockSink*>(this)->push(pBlock);
			}
		};

		InputSource GetRemoteMask() {
			return static_cast<InputSource>(
					utils::to_underlying_type(InputSource::Remote_Pull)
					| utils::to_underlying_type(InputSource::Remote_Push));
		}

		struct ConsumerTestContext {
		public:
			explicit ConsumerTestContext(InputSource sourceMask)
					: Consumer(CreateNewBlockConsumer(
							[&handler = NewBlockSink](const auto& pBlock) { handler(pBlock); },
							sourceMask))
			{}

		public:
			MockNewBlockSink NewBlockSink;
			disruptor::DisruptorConsumer Consumer;
		};

		ConsumerInput CreateInput(size_t numBlocks, InputSource source) {
			return test::CreateConsumerInputWithBlocks(numBlocks, source);
		}
	}

	TEST(TEST_CLASS, CanProcessZeroEntities) {
		ConsumerTestContext context(GetRemoteMask());
		test::AssertPassthroughForEmptyInput(context.Consumer);
	}

	namespace {
		void AssertBlockForwarded(ConsumerInput&& input) {
			// Arrange:
			ConsumerTestContext context(GetRemoteMask());
			auto pBlock = &input.blocks()[0].Block;

			// Act:
			auto result = context.Consumer(input);

			// Assert: the consumer detached the input
			test::AssertConsumed(result, validators::ValidationResult::Success);
			EXPECT_TRUE(input.empty());

			// - the block was passed to the callback (backed by original memory)
			const auto& params = context.NewBlockSink.params();
			ASSERT_EQ(1u, params.size());
			EXPECT_EQ(pBlock, params[0].pNewBlock.get());
		}

		void AssertBlockNotForwarded(ConsumerInput&& input) {
			// Arrange:
			ConsumerTestContext context(GetRemoteMask());

			// Act:
			auto result = context.Consumer(input);

			// Assert: the consumer did not detach the input
			test::AssertContinued(result);
			EXPECT_FALSE(input.empty());

			// - the block was not passed to the callback
			ASSERT_EQ(0u, context.NewBlockSink.params().size());
		}
	}

	TEST(TEST_CLASS, SingleBlockMatchingMaskIsForwarded) {
		AssertBlockForwarded(CreateInput(1, InputSource::Remote_Pull));
		AssertBlockForwarded(CreateInput(1, InputSource::Remote_Push));
	}

	TEST(TEST_CLASS, SingleBlockNotMatchingMaskIsNotForwarded) {
		AssertBlockNotForwarded(CreateInput(1, InputSource::Unknown));
		AssertBlockNotForwarded(CreateInput(1, InputSource::Local));
	}

	TEST(TEST_CLASS, MultipleBlocksMatchingMaskAreNotForwarded) {
		AssertBlockNotForwarded(CreateInput(3, InputSource::Remote_Pull));
		AssertBlockNotForwarded(CreateInput(3, InputSource::Remote_Push));
	}

	TEST(TEST_CLASS, MultipleBlocksNotMatchingMaskAreNotForwarded) {
		AssertBlockNotForwarded(CreateInput(3, InputSource::Unknown));
		AssertBlockNotForwarded(CreateInput(3, InputSource::Local));
	}
}}

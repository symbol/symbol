#include "catapult/consumers/BlockConsumers.h"
#include "tests/catapult/consumers/utils/ConsumerInputFactory.h"
#include "tests/catapult/consumers/utils/ConsumerTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/nodeps/ParamsCapture.h"
#include "tests/TestHarness.h"

using catapult::disruptor::ConsumerInput;
using catapult::disruptor::InputSource;

namespace catapult { namespace consumers {

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
					utils::to_underlying_type(InputSource::Remote_Pull) |
					utils::to_underlying_type(InputSource::Remote_Push));
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

	TEST(NewBlockConsumerTests, CanProcessZeroEntities) {
		// Assert:
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
			test::AssertConsumed(result);
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

	TEST(NewBlockConsumerTests, SingleBlockMatchingMaskIsForwarded) {
		// Assert:
		AssertBlockForwarded(CreateInput(1, InputSource::Remote_Pull));
		AssertBlockForwarded(CreateInput(1, InputSource::Remote_Push));
	}

	TEST(NewBlockConsumerTests, SingleBlockNotMatchingMaskIsNotForwarded) {
		// Assert:
		AssertBlockNotForwarded(CreateInput(1, InputSource::Unknown));
		AssertBlockNotForwarded(CreateInput(1, InputSource::Local));
	}

	TEST(NewBlockConsumerTests, MultipleBlocksMatchingMaskAreNotForwarded) {
		// Assert:
		AssertBlockNotForwarded(CreateInput(3, InputSource::Remote_Pull));
		AssertBlockNotForwarded(CreateInput(3, InputSource::Remote_Push));
	}

	TEST(NewBlockConsumerTests, MultipleBlocksNotMatchingMaskAreNotForwarded) {
		// Assert:
		AssertBlockNotForwarded(CreateInput(3, InputSource::Unknown));
		AssertBlockNotForwarded(CreateInput(3, InputSource::Local));
	}
}}

#include "catapult/consumers/ReclaimMemoryInspector.h"
#include "tests/catapult/consumers/test/ConsumerInputFactory.h"
#include "tests/TestHarness.h"

namespace catapult { namespace consumers {

#define TEST_CLASS ReclaimMemoryInspectorTests

	namespace {
		using disruptor::ConsumerInput;

		void AssertReclaimedMemory(const ConsumerInput& input) {
			EXPECT_TRUE(input.empty());
		}
	}

	TEST(TEST_CLASS, CanProcessZeroEntities) {
		// Arrange:
		ConsumerInput input;

		// Act:
		CreateReclaimMemoryInspector()(input, disruptor::ConsumerCompletionResult());

		// Assert:
		AssertReclaimedMemory(input);
	}

	namespace {
		void AssertCanReclaimMemory(ConsumerInput&& input) {
			// Sanity:
			EXPECT_FALSE(input.empty());

			// Act:
			CreateReclaimMemoryInspector()(input, disruptor::ConsumerCompletionResult());

			// Assert:
			AssertReclaimedMemory(input);
		}

		struct BlockTraits {
			static auto CreateInput(size_t numBlocks) {
				return test::CreateConsumerInputWithBlocks(numBlocks, disruptor::InputSource::Unknown);
			}
		};

		struct TransactionTraits {
			static auto CreateInput(size_t numTransactions) {
				return test::CreateConsumerInputWithTransactions(numTransactions, disruptor::InputSource::Unknown);
			}
		};

#define ENTITY_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Block) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BlockTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Transaction) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransactionTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()
	}

	ENTITY_TRAITS_BASED_TEST(CanProcessSingleEntity) {
		// Assert:
		AssertCanReclaimMemory(TTraits::CreateInput(1));
	}

	ENTITY_TRAITS_BASED_TEST(CanProcessMultipleEntities) {
		// Assert:
		AssertCanReclaimMemory(TTraits::CreateInput(3));
	}
}}

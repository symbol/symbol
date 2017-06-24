#include "catapult/disruptor/ConsumerInput.h"
#include "tests/catapult/disruptor/utils/ConsumerInputTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace disruptor {

#define TEST_CLASS ConsumerInputTests

	namespace {
		struct BlockTraits : public test::BlockTraits {
		public:
			using test::BlockTraits::CreateInput;

			static ConsumerInput CreateInput(size_t numEntities, InputSource source, test::EntitiesVector& entities) {
				auto range = test::CreateBlockEntityRange(numEntities);
				entities = test::ExtractEntities(range);
				return ConsumerInput(std::move(range), source);
			}

			static auto DetachRange(ConsumerInput& input) {
				return input.detachBlockRange();
			}

			static void AssertConsumerInputCreation(size_t numBlocks) {
				// Act:
				test::EntitiesVector entities;
				auto input = CreateInput(numBlocks, entities);

				// Assert:
				AssertInput(input, numBlocks, entities, InputSource::Unknown);
			}
		};

		struct TransactionTraits : public test::TransactionTraits {
		public:
			using test::TransactionTraits::CreateInput;

			static ConsumerInput CreateInput(size_t numEntities, InputSource source, test::EntitiesVector& entities) {
				auto range = test::CreateTransactionEntityRange(numEntities);
				entities = test::ExtractEntities(range);
				return ConsumerInput(std::move(range), source);
			}

			static auto DetachRange(ConsumerInput& input) {
				return input.detachTransactionRange();
			}

			static void AssertConsumerInputCreation(size_t numTransactions) {
				// Act:
				test::EntitiesVector entities;
				auto input = CreateInput(numTransactions, entities);

				// Assert:
				AssertInput(input, numTransactions, entities, InputSource::Unknown);
			}
		};
	}

#define ENTITY_TRAITS_BASED_TEST(TEST_NAME) ENTITY_TRAITS_BASED_CLASS_TEST(TEST_CLASS, TEST_NAME)

	TEST(TEST_CLASS, CanCreateEmptyConsumerInput) {
		// Act:
		ConsumerInput input;

		// Assert:
		test::AssertEmptyInput(input);
	}

	ENTITY_TRAITS_BASED_TEST(CanCreateConsumerInputWithCustomSource) {
		// Arrange:
		test::EntitiesVector entities;
		auto input = TTraits::CreateInput(3, InputSource::Local, entities);

		// Assert:
		TTraits::AssertInput(input, 3, entities, InputSource::Local);
	}

	ENTITY_TRAITS_BASED_TEST(CanCreateConsumerInputAroundSingleEntity) {
		// Assert:
		TTraits::AssertConsumerInputCreation(1);
	}

	ENTITY_TRAITS_BASED_TEST(CanCreateConsumerInputAroundMultipleEntities) {
		// Assert:
		TTraits::AssertConsumerInputCreation(3);
	}

	ENTITY_TRAITS_BASED_TEST(CanDetachMatchingEntitiesFromInput) {
		// Arrange:
		test::EntitiesVector entities;
		auto input = TTraits::CreateInput(2, entities);

		// Sanity:
		EXPECT_FALSE(input.empty());
		ASSERT_EQ(2u, entities.size());

		// Act:
		auto detachedEntities = TTraits::DetachRange(input);

		// Assert:
		EXPECT_TRUE(input.empty());
		ASSERT_EQ(2u, detachedEntities.size());

		auto iter = detachedEntities.cbegin();
		for (auto i = 0u; i < entities.size(); ++i) {
			EXPECT_EQ(entities[i], &*iter) << "entity at " << i;
			++iter;
		}
	}

	ENTITY_TRAITS_BASED_TEST(CannotDetachEntitiesFromEmptyInput) {
		// Arrange:
		auto input = ConsumerInput();

		// Act:
		EXPECT_THROW(TTraits::DetachRange(input), catapult_runtime_error);
	}

	ENTITY_TRAITS_BASED_TEST(CanOnlyDetachEntitiesFromInputOnce) {
		// Arrange:
		test::EntitiesVector entities;
		auto input = TTraits::CreateInput(2, entities);

		// Act: the first detach should succeed
		TTraits::DetachRange(input);

		// Assert: subsequent detaches should fail
		EXPECT_THROW(TTraits::DetachRange(input), catapult_runtime_error);
		EXPECT_THROW(TTraits::DetachRange(input), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanOutputEmptyConsumerInput) {
		// Arrange:
		ConsumerInput input;

		// Act:
		auto str = test::ToString(input);

		// Assert:
		EXPECT_EQ("empty from Unknown", str);
	}

	namespace {
		ConsumerInput PrepareBlockConsumerInputForOutputTests(Height startHeight) {
			// Arrange:
			auto pBlock1 = test::GenerateEmptyRandomBlock();
			auto pBlock2 = test::GenerateEmptyRandomBlock();
			auto pBlock3 = test::GenerateEmptyRandomBlock();
			pBlock1->Height = startHeight;
			pBlock2->Height = startHeight + Height(1);
			pBlock3->Height = startHeight + Height(2);
			auto range = test::CreateEntityRange({ pBlock1.get(), pBlock2.get(), pBlock3.get() });
			ConsumerInput input(std::move(range), InputSource::Remote_Push);
			input.blocks()[0].EntityHash = { { 0x98, 0xD6, 0xF4, 0xC2, 0xFF } };
			return input;
		}
	}

	TEST(TEST_CLASS, CanOutputBlockConsumerInput) {
		// Arrange:
		auto input = PrepareBlockConsumerInputForOutputTests(Height(12345));

		// Act:
		auto str = test::ToString(input);

		// Assert:
		EXPECT_EQ("3 blocks (heights 12345 - 12347) [98D6F4C2] from Remote_Push", str);
	}

	TEST(TEST_CLASS, CanOutputBlockConsumerInputWithDetachedRange) {
		// Arrange:
		auto input = PrepareBlockConsumerInputForOutputTests(Height(12345));
		auto range = input.detachBlockRange();

		// Act:
		auto str = test::ToString(input);

		// Assert:
		EXPECT_EQ("3 blocks (heights 12345 - 12347) [98D6F4C2] empty from Remote_Push", str);
	}

	namespace {
		ConsumerInput PrepareTransactionConsumerInputForOutputTests() {
			// Arrange:
			auto pTransaction1 = test::GenerateRandomTransaction();
			auto pTransaction2 = test::GenerateRandomTransaction();
			auto range = test::CreateEntityRange(std::vector<const model::Transaction*>{ pTransaction1.get(), pTransaction2.get() });
			ConsumerInput input(std::move(range), InputSource::Remote_Pull);
			input.transactions()[0].EntityHash = { { 0x00, 0xDA, 0x28, 0x96, 0xFF } };
			return input;
		}
	}

	TEST(TEST_CLASS, CanOutputTransactionConsumerInput) {
		// Arrange:
		auto input = PrepareTransactionConsumerInputForOutputTests();

		// Act:
		auto str = test::ToString(input);

		// Assert:
		EXPECT_EQ("2 txes [00DA2896] from Remote_Pull", str);
	}

	TEST(TEST_CLASS, CanOutputTransactionConsumerInputWithDetachedRange) {
		// Arrange:
		auto input = PrepareTransactionConsumerInputForOutputTests();
		input.detachTransactionRange();

		// Act:
		auto str = test::ToString(input);

		// Assert:
		EXPECT_EQ("2 txes [00DA2896] empty from Remote_Pull", str);
	}
}}

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

#include "catapult/disruptor/ConsumerInput.h"
#include "tests/catapult/disruptor/test/ConsumerInputTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace disruptor {

#define TEST_CLASS ConsumerInputTests

	namespace {
		// region BlockTraits

		struct BlockTraits : public test::BlockTraits {
		public:
			using test::BlockTraits::CreateInput;

			static model::BlockRange CreateRange(size_t numEntities, test::EntitiesVector& entities) {
				auto range = test::CreateBlockEntityRange(numEntities);
				entities = test::ExtractEntities(range);
				return range;
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

		// endregion

		// region TransactionTraits

		struct TransactionTraits : public test::TransactionTraits {
		public:
			using test::TransactionTraits::CreateInput;

			static model::TransactionRange CreateRange(size_t numEntities, test::EntitiesVector& entities) {
				auto range = test::CreateTransactionEntityRange(numEntities);
				entities = test::ExtractEntities(range);
				return range;
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

		// endregion
	}

#define ENTITY_TRAITS_BASED_TEST(TEST_NAME) ENTITY_TRAITS_BASED_CLASS_TEST(TEST_CLASS, TEST_NAME)

	// region constructor

	TEST(TEST_CLASS, CanCreateEmptyConsumerInput) {
		// Act:
		ConsumerInput input;

		// Assert:
		test::AssertEmptyInput(input);
	}

	ENTITY_TRAITS_BASED_TEST(CanCreateConsumerInputWithCustomSource) {
		// Arrange:
		test::EntitiesVector entities;
		auto range = TTraits::CreateRange(3, entities);
		auto input = ConsumerInput(std::move(range), InputSource::Local);

		// Assert:
		TTraits::AssertInput(input, 3, entities, InputSource::Local);
	}

	ENTITY_TRAITS_BASED_TEST(CanCreateConsumerInputWithCustomContext) {
		// Arrange:
		test::EntitiesVector entities;
		auto identityKey = test::GenerateRandomByteArray<Key>();
		auto range = TTraits::CreateRange(3, entities);
		auto input = ConsumerInput({ std::move(range), { identityKey, "11.22.33.44" } }, InputSource::Local);

		// Assert:
		TTraits::AssertInput(input, 3, entities, InputSource::Local);
		EXPECT_EQ(identityKey, input.sourceIdentity().PublicKey);
		EXPECT_EQ("11.22.33.44", input.sourceIdentity().Host);
	}

	ENTITY_TRAITS_BASED_TEST(CanCreateConsumerInputAroundSingleEntity) {
		TTraits::AssertConsumerInputCreation(1);
	}

	ENTITY_TRAITS_BASED_TEST(CanCreateConsumerInputAroundMultipleEntities) {
		TTraits::AssertConsumerInputCreation(3);
	}

	// endregion

	// region memorySize

	TEST(TEST_CLASS, CanCalculateMemorySizeForEmptyInput) {
		// Act:
		ConsumerInput input;

		// Assert:
		EXPECT_EQ(utils::FileSize(), input.memorySize());
	}

	TEST(TEST_CLASS, CanCalculateMemorySizeForBlockConsumerInput) {
		// Arrange:
		auto pBlock1 = test::GenerateEmptyRandomBlock();
		pBlock1->Size = 123;

		auto pBlock2 = test::GenerateEmptyRandomBlock();
		pBlock2->Size = 500;

		auto pBlock3 = test::GenerateEmptyRandomBlock();
		pBlock3->Size = 222;

		auto range = test::CreateEntityRange({ pBlock1.get(), pBlock2.get(), pBlock3.get() });
		ConsumerInput input(std::move(range), InputSource::Remote_Push);

		// Act:
		auto memorySize = input.memorySize();

		// Assert:
		EXPECT_EQ(utils::FileSize::FromBytes(845), memorySize);
	}

	TEST(TEST_CLASS, CanCalculateMemorySizeForTransactionConsumerInput) {
		// Arrange:
		auto pTransaction1 = test::GenerateRandomTransaction();
		pTransaction1->Size = 123;

		auto pTransaction2 = test::GenerateRandomTransaction();
		pTransaction2->Size = 500;

		auto range = test::CreateEntityRange(std::vector<const model::Transaction*>{ pTransaction1.get(), pTransaction2.get() });
		ConsumerInput input(std::move(range), InputSource::Remote_Pull);

		// Act:
		auto memorySize = input.memorySize();

		// Assert:
		EXPECT_EQ(utils::FileSize::FromBytes(623), memorySize);
	}

	// endregion

	// region detach range

	ENTITY_TRAITS_BASED_TEST(CanDetachMatchingEntitiesFromInput) {
		// Arrange:
		test::EntitiesVector entities;
		auto input = TTraits::CreateInput(2, entities);

		// Sanity:
		EXPECT_FALSE(input.empty());
		EXPECT_TRUE(input.hasBlocks() || input.hasTransactions());
		ASSERT_EQ(2u, entities.size());

		// Act:
		auto detachedEntities = TTraits::DetachRange(input);

		// Assert:
		EXPECT_TRUE(input.empty());
		EXPECT_FALSE(input.hasBlocks() || input.hasTransactions());
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

		// Act + Assert:
		EXPECT_THROW(TTraits::DetachRange(input), catapult_runtime_error);
	}

	ENTITY_TRAITS_BASED_TEST(CanOnlyDetachEntitiesFromInputOnce) {
		// Arrange:
		test::EntitiesVector entities;
		auto input = TTraits::CreateInput(2, entities);

		// Act: the first detach should succeed
		TTraits::DetachRange(input);

		// Act + Assert: subsequent detaches should fail
		EXPECT_THROW(TTraits::DetachRange(input), catapult_runtime_error);
		EXPECT_THROW(TTraits::DetachRange(input), catapult_runtime_error);
	}

	// endregion

	// region insertion operator

	TEST(TEST_CLASS, CanOutputEmptyConsumerInput) {
		// Arrange:
		ConsumerInput input;

		// Act:
		auto str = test::ToString(input);

		// Assert:
		EXPECT_EQ("empty from Unknown with size 0B", str);
	}

	namespace {
		ConsumerInput PrepareBlockConsumerInputForOutputTests(Height startHeight) {
			// Arrange:
			auto pBlock1 = test::GenerateEmptyRandomBlock();
			pBlock1->Size = 123;
			pBlock1->Height = startHeight;

			auto pBlock2 = test::GenerateEmptyRandomBlock();
			pBlock2->Size = 500;
			pBlock2->Height = startHeight + Height(1);

			auto pBlock3 = test::GenerateEmptyRandomBlock();
			pBlock3->Size = 222;
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
		EXPECT_EQ("3 blocks (heights 12345 - 12347) [98D6F4C2] from Remote_Push with size 845B", str);
	}

	TEST(TEST_CLASS, CanOutputBlockConsumerInputWithDetachedRange) {
		// Arrange:
		auto input = PrepareBlockConsumerInputForOutputTests(Height(12345));
		{
			auto range = input.detachBlockRange();
		}

		// Sanity:
		EXPECT_TRUE(input.empty());

		// Act:
		auto str = test::ToString(input);

		// Assert:
		EXPECT_EQ("3 blocks (heights 12345 - 12347) [98D6F4C2] empty from Remote_Push with size 845B", str);
	}

	namespace {
		ConsumerInput PrepareTransactionConsumerInputForOutputTests() {
			// Arrange:
			auto pTransaction1 = test::GenerateRandomTransaction();
			pTransaction1->Size = 123;

			auto pTransaction2 = test::GenerateRandomTransaction();
			pTransaction2->Size = 500;

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
		EXPECT_EQ("2 txes [00DA2896] from Remote_Pull with size 623B", str);
	}

	TEST(TEST_CLASS, CanOutputTransactionConsumerInputWithDetachedRange) {
		// Arrange:
		auto input = PrepareTransactionConsumerInputForOutputTests();
		{
			auto range = input.detachTransactionRange();
		}

		// Sanity:
		EXPECT_TRUE(input.empty());

		// Act:
		auto str = test::ToString(input);

		// Assert:
		EXPECT_EQ("2 txes [00DA2896] empty from Remote_Pull with size 623B", str);
	}

	// endregion
}}

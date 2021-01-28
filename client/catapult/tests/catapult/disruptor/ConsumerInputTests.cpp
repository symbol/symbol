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
		// region traits + utils

		struct BlockTraits : public test::BlockTraits {
			using EntityType = model::Block;

			static auto GenerateRandomEntity(uint32_t size) {
				return test::GenerateBlockWithTransactions(test::ConstTransactions{ test::GenerateRandomTransactionWithSize(size) });
			}

			static auto DetachRange(ConsumerInput& input) {
				return input.detachBlockRange();
			}
		};

		struct TransactionTraits : public test::TransactionTraits {
			using EntityType = model::Transaction;

			static constexpr auto GenerateRandomEntity = test::GenerateRandomTransactionWithSize;

			static auto DetachRange(ConsumerInput& input) {
				return input.detachTransactionRange();
			}
		};

		template<typename TTraits>
		auto CreateEntityRange(std::initializer_list<uint32_t> sizes) {
			std::vector<std::unique_ptr<typename TTraits::EntityType>> entities;
			std::vector<const typename TTraits::EntityType*> entityPointers;

			for (auto size : sizes) {
				entities.push_back(TTraits::GenerateRandomEntity(size));

				entityPointers.push_back(entities.back().get());
			}

			return test::CreateEntityRange(entityPointers);
		}

		// endregion
	}

#define ENTITY_TRAITS_BASED_TEST(TEST_NAME) ENTITY_TRAITS_BASED_CLASS_TEST(TEST_CLASS, TEST_NAME)

	// region constructor

	namespace {
		template<typename TTraits>
		void AssertConsumerInputCreation(std::initializer_list<uint32_t> sizes) {
			// Arrange:
			auto range = CreateEntityRange<TTraits>(sizes);
			auto entities = test::ExtractEntities(range);

			// Act:
			auto input = ConsumerInput(std::move(range));

			// Assert:
			auto expectedCount = static_cast<size_t>(std::distance(sizes.begin(), sizes.end()));
			TTraits::AssertInput(input, expectedCount, entities, InputSource::Unknown);
		}
	}

	TEST(TEST_CLASS, CanCreateEmptyConsumerInput) {
		// Act:
		ConsumerInput input;

		// Assert:
		test::AssertEmptyInput(input);
	}

	ENTITY_TRAITS_BASED_TEST(CanCreateConsumerInputWithCustomSource) {
		// Arrange:
		auto range = CreateEntityRange<TTraits>({ 143, 143, 143 });
		auto entities = test::ExtractEntities(range);

		// Act:
		auto input = ConsumerInput(std::move(range), InputSource::Local);

		// Assert:
		TTraits::AssertInput(input, 3, entities, InputSource::Local);
	}

	ENTITY_TRAITS_BASED_TEST(CanCreateConsumerInputWithCustomContext) {
		// Arrange:
		auto identityKey = test::GenerateRandomByteArray<Key>();
		auto range = CreateEntityRange<TTraits>({ 143, 143, 143 });
		auto entities = test::ExtractEntities(range);

		// Act:
		auto input = ConsumerInput({ std::move(range), { identityKey, "11.22.33.44" } }, InputSource::Local);

		// Assert:
		TTraits::AssertInput(input, 3, entities, InputSource::Local);
		EXPECT_EQ(identityKey, input.sourceIdentity().PublicKey);
		EXPECT_EQ("11.22.33.44", input.sourceIdentity().Host);
	}

	ENTITY_TRAITS_BASED_TEST(CanCreateConsumerInputAroundSingleEntity) {
		AssertConsumerInputCreation<TTraits>({ 143 });
	}

	ENTITY_TRAITS_BASED_TEST(CanCreateConsumerInputAroundMultipleEntities) {
		AssertConsumerInputCreation<TTraits>({ 143, 143, 143 });
	}

	// endregion

	// region memorySize

	TEST(TEST_CLASS, CanCalculateMemorySizeForEmptyInput) {
		// Arrange:
		ConsumerInput input;

		// Act:
		auto memorySize = input.memorySize();

		// Assert:
		EXPECT_EQ(utils::FileSize(), memorySize);
	}

	TEST(TEST_CLASS, CanCalculateMemorySizeForBlockConsumerInput) {
		// Arrange:
		auto range = CreateEntityRange<BlockTraits>({ 143, 500, 222 });
		ConsumerInput input(std::move(range), InputSource::Remote_Push);

		// Act:
		auto memorySize = input.memorySize();

		// Assert:
		EXPECT_EQ(utils::FileSize::FromBytes(3 * 376 + 865), memorySize);
	}

	TEST(TEST_CLASS, CanCalculateMemorySizeForTransactionConsumerInput) {
		// Arrange:
		auto range = CreateEntityRange<TransactionTraits>({ 143, 500 });
		ConsumerInput input(std::move(range), InputSource::Remote_Pull);

		// Act:
		auto memorySize = input.memorySize();

		// Assert:
		EXPECT_EQ(utils::FileSize::FromBytes(643), memorySize);
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
			auto range = CreateEntityRange<BlockTraits>({ 143, 500, 222 });

			auto height = startHeight;
			for (auto& block : range) {
				block.Height = height;
				height = height + Height(1);
			}

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
		EXPECT_EQ("3 blocks (heights 12345 - 12347) [98D6F4C2] from Remote_Push with size 1KB 969B", str);
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
		EXPECT_EQ("3 blocks (heights 12345 - 12347) [98D6F4C2] empty from Remote_Push with size 1KB 969B", str);
	}

	namespace {
		ConsumerInput PrepareTransactionConsumerInputForOutputTests() {
			auto range = CreateEntityRange<TransactionTraits>({ 143, 500 });
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
		EXPECT_EQ("2 txes [00DA2896] from Remote_Pull with size 643B", str);
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
		EXPECT_EQ("2 txes [00DA2896] empty from Remote_Pull with size 643B", str);
	}

	// endregion
}}

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

#pragma once
#include "catapult/disruptor/ConsumerInput.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Vector of verifiable entities.
	using EntitiesVector = std::vector<const model::VerifiableEntity*>;

	/// Asserts that \a input is empty.
	void AssertEmptyInput(disruptor::ConsumerInput& input);

	/// Extracts entities from \a range into a vector.
	template<typename TEntity>
	EntitiesVector ExtractEntities(const model::EntityRange<TEntity>& range) {
		EntitiesVector entities;
		for (const auto& entity : range)
			entities.push_back(&entity);

		return entities;
	}

	/// Traits for testing a block-based input.
	struct BlockTraits {
	public:
		/// Creates an input with \a numEntities blocks and copies the entities into \a entities.
		static disruptor::ConsumerInput CreateInput(size_t numEntities, EntitiesVector& entities) {
			auto range = test::CreateBlockEntityRange(numEntities);
			entities = ExtractEntities(range);
			return disruptor::ConsumerInput(std::move(range));
		}

		/// Asserts that \a input has \a numBlocks blocks matching \a entities with a source of \a expectedInputSource.
		static void AssertInput(
				disruptor::ConsumerInput& input,
				size_t numBlocks,
				const std::vector<const model::VerifiableEntity*>& entities,
				disruptor::InputSource expectedInputSource) {
			CATAPULT_LOG(debug) << "checking const input";
			AssertInputT<const disruptor::ConsumerInput>(input, numBlocks, entities, expectedInputSource);

			CATAPULT_LOG(debug) << "checking non const input";
			AssertInputT<disruptor::ConsumerInput>(input, numBlocks, entities, expectedInputSource);
		}

	private:
		template<typename TInput>
		static void AssertInputT(
				TInput& input,
				size_t numBlocks,
				const std::vector<const model::VerifiableEntity*>& entities,
				disruptor::InputSource expectedInputSource) {
			// Assert:
			EXPECT_FALSE(input.empty());
			EXPECT_TRUE(input.hasBlocks());
			EXPECT_FALSE(input.hasTransactions());

			EXPECT_EQ(expectedInputSource, input.source());
			EXPECT_THROW(input.transactions(), catapult_runtime_error);

			EXPECT_EQ(numBlocks, input.blocks().size());
			EXPECT_EQ(entities.size(), input.blocks().size());
			for (auto i = 0u; i < entities.size(); ++i)
				EXPECT_EQ(entities[i], &input.blocks()[i].Block) << "block at " << i;
		}
	};

	/// Traits for testing a transaction-based input.
	struct TransactionTraits {
	public:
		/// Creates an input with \a numEntities transactions and copies the entities into \a entities.
		static disruptor::ConsumerInput CreateInput(size_t numEntities, EntitiesVector& entities) {
			auto range = test::CreateTransactionEntityRange(numEntities);
			entities = ExtractEntities(range);
			return disruptor::ConsumerInput(std::move(range));
		}

		/// Asserts that \a input has \a numTransactions transactions matching \a entities with a source of \a expectedInputSource.
		static void AssertInput(
				disruptor::ConsumerInput& input,
				size_t numTransactions,
				const std::vector<const model::VerifiableEntity*>& entities,
				disruptor::InputSource expectedInputSource) {
			CATAPULT_LOG(debug) << "checking const input";
			AssertInputT<const disruptor::ConsumerInput>(input, numTransactions, entities, expectedInputSource);

			CATAPULT_LOG(debug) << "checking non const input";
			AssertInputT<disruptor::ConsumerInput>(input, numTransactions, entities, expectedInputSource);
		}

	private:
		template<typename TInput>
		static void AssertInputT(
				TInput& input,
				size_t numTransactions,
				const std::vector<const model::VerifiableEntity*>& entities,
				disruptor::InputSource expectedInputSource) {
			// Assert:
			EXPECT_FALSE(input.empty());
			EXPECT_FALSE(input.hasBlocks());
			EXPECT_TRUE(input.hasTransactions());

			EXPECT_EQ(expectedInputSource, input.source());
			EXPECT_THROW(input.blocks(), catapult_runtime_error);

			EXPECT_EQ(numTransactions, input.transactions().size());
			EXPECT_EQ(entities.size(), input.transactions().size());
			for (auto i = 0u; i < entities.size(); ++i)
				EXPECT_EQ(entities[i], &input.transactions()[i].Transaction) << "transaction at " << i;
		}
	};

/// Defines an entity traits based test named \a TEST_NAME in \a TEST_CLASS.
#define ENTITY_TRAITS_BASED_CLASS_TEST(TEST_CLASS, TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Block) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BlockTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Transaction) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransactionTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()
}}

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
		AssertCanReclaimMemory(TTraits::CreateInput(1));
	}

	ENTITY_TRAITS_BASED_TEST(CanProcessMultipleEntities) {
		AssertCanReclaimMemory(TTraits::CreateInput(3));
	}
}}

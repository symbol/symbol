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

#include "catapult/disruptor/DisruptorElement.h"
#include "tests/catapult/disruptor/test/ConsumerInputTestUtils.h"
#include "tests/test/other/DisruptorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace disruptor {

#define TEST_CLASS DisruptorElementTests

	namespace {
		void EmptyProcessingCompleteFunc(DisruptorElementId, ConsumerResult)
		{}

		ConsumerResult CreateConsumerResult(uint32_t code, uint8_t severity) {
			ConsumerResult result;
			result.CompletionCode = code;
			result.ResultSeverity = static_cast<ConsumerResultSeverity>(severity);
			return result;
		}

		struct BlockTraits : public test::BlockTraits {
		public:
			static void AssertDisruptorElementCreation(size_t numBlocks) {
				// Act::
				test::EntitiesVector entities;
				auto element = DisruptorElement(CreateInput(numBlocks, entities), 17, EmptyProcessingCompleteFunc);

				// Assert:
				AssertInput(element.input(), numBlocks, entities, InputSource::Unknown);
				EXPECT_EQ(17u, element.id());
				EXPECT_FALSE(element.isSkipped());
			}
		};

		struct TransactionTraits : public test::TransactionTraits {
		public:
			static void AssertDisruptorElementCreation(size_t numTransactions) {
				// Act:
				test::EntitiesVector entities;
				auto element = DisruptorElement(CreateInput(numTransactions, entities), 17, EmptyProcessingCompleteFunc);

				// Assert:
				AssertInput(element.input(), numTransactions, entities, InputSource::Unknown);
				EXPECT_EQ(17u, element.id());
				EXPECT_FALSE(element.isSkipped());
			}
		};
	}

#define ENTITY_TRAITS_BASED_TEST(TEST_NAME) ENTITY_TRAITS_BASED_CLASS_TEST(TEST_CLASS, TEST_NAME)

	// region DisruptorElement

	TEST(TEST_CLASS, CanCreateEmptyDisruptorElement) {
		// Act:
		DisruptorElement element;

		// Assert:
		test::AssertEmptyInput(element.input());
		EXPECT_EQ(static_cast<uint64_t>(-1), element.id());
		EXPECT_FALSE(element.isSkipped());
		test::AssertContinued(element.completionResult());
	}

	ENTITY_TRAITS_BASED_TEST(CanCreateDisruptorElementAroundSingleEntity) {
		TTraits::AssertDisruptorElementCreation(1);
	}

	ENTITY_TRAITS_BASED_TEST(CanCreateDisruptorElementAroundMultipleEntities) {
		TTraits::AssertDisruptorElementCreation(3);
	}

	TEST(TEST_CLASS, CanMarkDisruptorElementAsSkipped) {
		// Arrange:
		DisruptorElement element;

		// Sanity:
		EXPECT_FALSE(element.isSkipped());

		// Act:
		element.markSkipped(7, CreateConsumerResult(9, 8));

		// Assert:
		EXPECT_TRUE(element.isSkipped());
		test::AssertAborted(element.completionResult(), 9, static_cast<ConsumerResultSeverity>(8), 7);
	}

	TEST(TEST_CLASS, CanOutputDisruptorElement) {
		// Arrange:
		auto pTransaction1 = test::GenerateRandomTransaction();
		auto pTransaction2 = test::GenerateRandomTransaction();
		auto range = test::CreateEntityRange(std::vector<const model::Transaction*>{ pTransaction1.get(), pTransaction2.get() });
		ConsumerInput input(std::move(range), InputSource::Remote_Pull);
		input.transactions()[0].EntityHash = { { 0x00, 0xDA, 0x28, 0x96, 0xFF } };
		DisruptorElement element(std::move(input), 21, EmptyProcessingCompleteFunc);

		// Act:
		auto str = test::ToString(element);

		// Assert:
		EXPECT_EQ("element 21 (2 txes [00DA2896] from Remote_Pull with size 348B)", str);
	}

	namespace {
		void AssertProcessingCompleteDelegatesToCompletionHandler(
				const consumer<DisruptorElement&>& elementModifier,
				const ConsumerCompletionResult& expectedResult) {
			// Arrange:
			DisruptorElementId elementId;
			ConsumerCompletionResult consumerResult;
			DisruptorElement element(ConsumerInput(), 21, [&elementId, &consumerResult](auto id, auto result) {
				elementId = id;
				consumerResult = result;
			});

			elementModifier(element);

			// Act:
			element.markProcessingComplete();

			// Assert:
			EXPECT_EQ(21u, element.id());
			test::AssertEqual(expectedResult, consumerResult);
		}
	}

	TEST(TEST_CLASS, MarkProcessingCompleteDelegatesToCompletionHandlerWhenElementIsNotSkipped) {
		AssertProcessingCompleteDelegatesToCompletionHandler([](const auto&) {}, ConsumerCompletionResult());
	}

	TEST(TEST_CLASS, MarkProcessingCompleteDelegatesToCompletionHandlerWhenElementIsSkipped) {
		// Arrange:
		auto expectedResult = ConsumerCompletionResult();
		expectedResult.CompletionStatus = CompletionStatus::Aborted;
		expectedResult.CompletionCode = 22;
		expectedResult.ResultSeverity = static_cast<ConsumerResultSeverity>(8);
		expectedResult.FinalConsumerPosition = 123;

		// Assert:
		AssertProcessingCompleteDelegatesToCompletionHandler(
				[](auto& element) { element.markSkipped(123, CreateConsumerResult(22, 8)); },
				expectedResult);
	}

	// endregion

	// region IsIntervalElementId

	TEST(TEST_CLASS, IsIntervalElementIdReturnsFalseWhenIntervalIsZero) {
		EXPECT_FALSE(IsIntervalElementId(4, 0));
		EXPECT_FALSE(IsIntervalElementId(7, 0));
		EXPECT_FALSE(IsIntervalElementId(8, 0));
		EXPECT_FALSE(IsIntervalElementId(9, 0));
	}

	TEST(TEST_CLASS, IsIntervalElementIdReturnsFalseWhenIdIsNotAMultipleOfInterval) {
		EXPECT_FALSE(IsIntervalElementId(4, 8));
		EXPECT_FALSE(IsIntervalElementId(7, 3));
		EXPECT_FALSE(IsIntervalElementId(8, 3));
		EXPECT_FALSE(IsIntervalElementId(9, 4));
	}

	TEST(TEST_CLASS, IsIntervalElementIdReturnsTrueWhenIdIsMultipleOfInterval) {
		EXPECT_TRUE(IsIntervalElementId(4, 4));
		EXPECT_TRUE(IsIntervalElementId(7, 7));
		EXPECT_TRUE(IsIntervalElementId(8, 2));
		EXPECT_TRUE(IsIntervalElementId(9, 3));
	}

	// endregion
}}

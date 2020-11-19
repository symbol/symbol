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

#include "catapult/disruptor/Disruptor.h"
#include "catapult/thread/IoThreadPool.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/nodeps/Waits.h"
#include "tests/test/other/DisruptorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace disruptor {

#define TEST_CLASS DisruptorTests

	namespace {
		DisruptorElementId PushBlock(
				Disruptor& disruptor,
				std::unique_ptr<model::Block>&& pBlock,
				const ProcessingCompleteFunc& processingComplete = [](auto, auto) {}) {
			return disruptor.add(ConsumerInput(model::BlockRange::FromEntity(std::move(pBlock))), processingComplete);
		}
	}

	TEST(TEST_CLASS, CanCreateEmptyDisruptor) {
		// Arrange:
		Disruptor disruptor(16);

		// Assert:
		EXPECT_EQ(0u, disruptor.size());
		EXPECT_EQ(0u, disruptor.added());
		EXPECT_EQ(16u, disruptor.capacity());
	}

	TEST(TEST_CLASS, CanPushEntityIntoDisruptor) {
		// Arrange:
		Disruptor disruptor(16);
		auto pBlock = test::GenerateEmptyRandomBlock();

		// Act:
		PushBlock(disruptor, std::move(pBlock));

		// Assert:
		EXPECT_EQ(1u, disruptor.size());
		EXPECT_EQ(1u, disruptor.added());
		EXPECT_EQ(16u, disruptor.capacity());
	}

	TEST(TEST_CLASS, PushOverwritesElementsWhenCapacityIsExceeded) {
		// Arrange:
		Disruptor disruptor(16);

		// Act:
		for (auto i = 0u; i < 20; ++i) {
			auto pBlock = test::GenerateEmptyRandomBlock();
			pBlock->Height = Height(i + 1);
			PushBlock(disruptor, std::move(pBlock));
		}

		// Assert:
		EXPECT_EQ(16u, disruptor.size());
		EXPECT_EQ(20u, disruptor.added());
		EXPECT_EQ(16u, disruptor.capacity());

		// probably not the best test, as it actually tests the underlying circular buffer
		std::vector<Height::ValueType> collectedHeights;
		for (auto i = 0u; i < 16; ++i)
			collectedHeights.push_back(disruptor.elementAt(i).input().blocks()[0].Block.Height.unwrap());

		std::vector<Height::ValueType> expectedHeights{ 17, 18, 19, 20, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
		EXPECT_EQ(expectedHeights, collectedHeights);
	}

	namespace {
		void PrepareDisruptor(Disruptor& disruptor) {
			for (int i = 0; i < 20; ++i) {
				auto pBlock = test::GenerateEmptyRandomBlock();
				PushBlock(disruptor, std::move(pBlock));
			}
		}
	}

	TEST(TEST_CLASS, AddReturnsElementId) {
		// Arrange:
		Disruptor disruptor(16);
		auto pBlock1 = test::GenerateEmptyRandomBlock();
		auto pBlock2 = test::GenerateEmptyRandomBlock();

		// Act:
		auto id1 = PushBlock(disruptor, std::move(pBlock1));
		auto id2 = PushBlock(disruptor, std::move(pBlock2));

		// Assert:
		EXPECT_EQ(2u, disruptor.size());
		EXPECT_EQ(2u, disruptor.added());
		EXPECT_EQ(1u, id1);
		EXPECT_EQ(2u, id2);
		EXPECT_EQ(16u, disruptor.capacity());
	}

	namespace {
		ConsumerResult CreateConsumerResult(uint32_t code, uint8_t resultSeverity) {
			ConsumerResult result;
			result.CompletionCode = code;
			result.ResultSeverity = static_cast<ConsumerResultSeverity>(resultSeverity);
			return result;
		}

		template<typename TMarkElementAction, typename TValidateResultAction>
		void AssertAddCreatesElementWithExpectedIdAndCompletionHandler(
				TMarkElementAction markElement,
				TValidateResultAction validateResult) {
			// Arrange:
			Disruptor disruptor(32);
			PrepareDisruptor(disruptor);
			auto pBlock = test::GenerateEmptyRandomBlock();
			size_t callCount = 0;

			// Act:
			auto consumerResult = ConsumerCompletionResult();
			auto id = disruptor.add(ConsumerInput(model::BlockRange::FromEntity(std::move(pBlock))), [&](auto, auto result) {
				consumerResult = result;
				++callCount;
			});
			auto& element = disruptor.elementAt(20);

			// Sanity:
			EXPECT_EQ(0u, callCount);

			// - actually mark the element
			markElement(element);

			// Assert:
			EXPECT_EQ(21u, disruptor.size());
			EXPECT_EQ(21u, disruptor.added());
			EXPECT_EQ(21u, id);
			EXPECT_EQ(21u, element.id());
			EXPECT_EQ(1u, callCount);
			EXPECT_EQ(32u, disruptor.capacity());

			// - check the callback result
			validateResult(consumerResult);
		}
	}

	TEST(TEST_CLASS, AddCreatesElementWithExpectedIdAndCompletionHandler_ElementNotSkipped) {
		// Arrange:
		AssertAddCreatesElementWithExpectedIdAndCompletionHandler(
				[](auto& element) {
					// Act:
					element.markProcessingComplete();
				},
				[](auto& result) {
					// Assert:
					test::AssertContinued(result);
				});
	}

	TEST(TEST_CLASS, AddCreatesElementWithExpectedIdAndCompletionHandler_ElementSkipped) {
		// Arrange:
		AssertAddCreatesElementWithExpectedIdAndCompletionHandler(
				[](auto& element) {
					// Act:
					element.markSkipped(20, CreateConsumerResult(22, 17));
					element.markProcessingComplete();
				},
				[](auto& result) {
					// Assert:
					test::AssertAborted(result, 22, static_cast<disruptor::ConsumerResultSeverity>(17), 20);
				});
	}

	TEST(TEST_CLASS, CanMarkElements) {
		// Arrange:
		Disruptor disruptor(16);
		PrepareDisruptor(disruptor);

		// Act:
		for (auto i = 0u; i < 16; i += 2)
			disruptor.markSkipped(i, CreateConsumerResult(0, 1));

		// Assert:
		EXPECT_EQ(16u, disruptor.size());
		EXPECT_EQ(20u, disruptor.added());
		for (auto i = 0u; i < 16; ++i) {
			if (i % 2)
				EXPECT_FALSE(disruptor.isSkipped(i));
			else
				EXPECT_TRUE(disruptor.isSkipped(i));
		}
	}
}}

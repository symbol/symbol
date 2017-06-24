#include "catapult/disruptor/Disruptor.h"
#include "catapult/thread/IoServiceThreadPool.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/nodeps/Waits.h"
#include "tests/test/other/DisruptorTestUtils.h"
#include "tests/TestHarness.h"
#include <boost/thread.hpp>

namespace catapult { namespace disruptor {

	namespace {
		DisruptorElementId PushBlock(
				Disruptor& disruptor,
				std::unique_ptr<model::Block>&& pBlock,
				const ProcessingCompleteFunc& processingComplete = [](auto, auto) {}) {
			return disruptor.add(ConsumerInput(model::BlockRange::FromEntity(std::move(pBlock))), processingComplete);
		}
	}

	TEST(DisruptorTests, CanCreateEmptyDisruptor) {
		// Arrange:
		Disruptor disruptor(16);

		// Assert:
		EXPECT_EQ(0u, disruptor.size());
		EXPECT_EQ(0u, disruptor.added());
		EXPECT_EQ(16u, disruptor.capacity());
	}

	TEST(DisruptorTests, CanPushEntityIntoDisruptor) {
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

	TEST(DisruptorTests, PushOverwritesElementsIfCapacityIsExceeded) {
		// Arrange:
		Disruptor disruptor(16);

		// Act:
		for (auto i = 0u; i < 20u; ++i) {
			auto pBlock = test::GenerateEmptyRandomBlock();
			pBlock->Height = Height(i + 1u);
			PushBlock(disruptor, std::move(pBlock));
		}

		// Assert:
		EXPECT_EQ(16u, disruptor.size());
		EXPECT_EQ(20u, disruptor.added());
		EXPECT_EQ(16u, disruptor.capacity());

		// probably not the best test, as it actually tests the underlying circular buffer...
		std::vector<Height::ValueType> collectedHeights;
		for (auto i = 0u; i < 16u; ++i)
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

	TEST(DisruptorTests, AddReturnsElementId) {
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

	TEST(DisruptorTests, AddCreatesElementWithExpectedIdAndCompletionHandler_ElementNotSkipped) {
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

	TEST(DisruptorTests, AddCreatesElementWithExpectedIdAndCompletionHandler_ElementSkipped) {
		// Arrange:
		AssertAddCreatesElementWithExpectedIdAndCompletionHandler(
				[](auto& element) {
					// Act:
					element.markSkipped(20, 22);
					element.markProcessingComplete();
				},
				[](auto& result) {
					// Assert:
					test::AssertAborted(result, 22, 20);
				});
	}

	TEST(DisruptorTests, CanMarkElements) {
		// Arrange:
		Disruptor disruptor(16);
		PrepareDisruptor(disruptor);

		// Act:
		for (auto i = 0u; i < 16u; i += 2u)
			disruptor.markSkipped(i, 0);

		// Assert:
		EXPECT_EQ(16u, disruptor.size());
		EXPECT_EQ(20u, disruptor.added());
		for (auto i = 0u; i < 16u; ++i) {
			if (i % 2)
				EXPECT_FALSE(disruptor.isSkipped(i));
			else
				EXPECT_TRUE(disruptor.isSkipped(i));
		}
	}
}}

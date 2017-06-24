#include "catapult/disruptor/ConsumerDispatcher.h"
#include "catapult/model/RangeTypes.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/nodeps/Atomics.h"
#include "tests/test/nodeps/Functional.h"
#include "tests/test/other/DisruptorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace disruptor {

	namespace {
		static constexpr ConsumerDispatcherOptions Test_Dispatcher_Options{ "ConsumerDispatcherTests", 16u * 1024 };

		auto CreateNoOpConsumer() {
			return [](ConsumerInput&) {
				return ConsumerResult::Continue();
			};
		}
	}

	TEST(ConsumerDispatcherTests, CannotCreateDispatcherWithNullName) {
		// Arrange:
		auto options = Test_Dispatcher_Options;
		options.DispatcherName = nullptr;

		// Assert:
		EXPECT_THROW(ConsumerDispatcher(options, {}), catapult_invalid_argument);
	}

	TEST(ConsumerDispatcherTests, CannotCreateDispatcherWithZeroSizeDisruptor) {
		// Arrange:
		auto options = Test_Dispatcher_Options;
		options.DisruptorSize = 0;

		// Assert:
		EXPECT_THROW(ConsumerDispatcher(options, {}), catapult_invalid_argument);
	}

	TEST(ConsumerDispatcherTests, CanCreateEmptyDispatcher) {
		// Arrange:
		ConsumerDispatcher dispatcher(Test_Dispatcher_Options, {});

		// Assert:
		EXPECT_EQ(Test_Dispatcher_Options.DispatcherName, dispatcher.name());
		EXPECT_EQ(0u, dispatcher.size());
		EXPECT_EQ(0u, dispatcher.numAddedElements());
		EXPECT_TRUE(dispatcher.isRunning());
	}

	TEST(ConsumerDispatcherTests, CanCreateDispatcherWithConsumer) {
		// Arrange + Act:
		ConsumerDispatcher dispatcher(Test_Dispatcher_Options, { CreateNoOpConsumer() });

		// Assert:
		EXPECT_EQ(Test_Dispatcher_Options.DispatcherName, dispatcher.name());
		EXPECT_EQ(1u, dispatcher.size());
		EXPECT_EQ(0u, dispatcher.numAddedElements());
		EXPECT_TRUE(dispatcher.isRunning());
	}

	namespace {
		auto CreateAlternatingResultConsumer() {
			auto pCounter = std::make_shared<uint8_t>(0);
			return [pCounter](const auto&) {
				auto id = ++*pCounter;
				return 0 == id % 2 ? ConsumerResult::Continue() : ConsumerResult::Abort(id * id);
			};
		}
	}

	TEST(ConsumerDispatcherTests, CompletionHandlerIsCalledWhenProcessingIsComplete) {
		// Arrange:
		ConsumerDispatcher dispatcher(Test_Dispatcher_Options, { CreateAlternatingResultConsumer() });
		auto ranges = test::PrepareRanges(3);
		std::vector<DisruptorElementId> elementIds;
		std::vector<DisruptorElementId> capturedElementIds;
		std::vector<ConsumerCompletionResult> capturedResults;

		// Act:
		std::atomic<size_t> numCapturedElements(0);
		for (auto i = 0u; i < ranges.size(); ++i) {
			// note: the alternating result consumer starts with ConsumerResult::Abort()
			elementIds.push_back(dispatcher.processElement(
					ConsumerInput(std::move(ranges[i])),
					[&capturedElementIds, &capturedResults, &numCapturedElements](auto id, const auto& result) {
						capturedElementIds.push_back(id);
						capturedResults.push_back(result);
						++numCapturedElements;
					}));
		}

		WAIT_FOR_VALUE(numCapturedElements, ranges.size());

		// Assert:
		std::vector<DisruptorElementId> expectedElementIds{ 1u, 2u, 3u };
		EXPECT_EQ(Test_Dispatcher_Options.DispatcherName, dispatcher.name());
		EXPECT_EQ(1u, dispatcher.size());
		EXPECT_EQ(3u, dispatcher.numAddedElements());
		EXPECT_TRUE(dispatcher.isRunning());
		EXPECT_EQ(expectedElementIds, elementIds);
		EXPECT_EQ(expectedElementIds, capturedElementIds);
		EXPECT_EQ(ranges.size(), capturedElementIds.size());

		ASSERT_EQ(ranges.size(), capturedResults.size());
		test::AssertAborted(capturedResults[0], 1, 0);
		test::AssertContinued(capturedResults[1]);
		test::AssertAborted(capturedResults[2], 9, 2);
	}

	TEST(ConsumerDispatcherTests, CompletionHandlerIsNotCalledWhenInputIsEmpty) {
		// Arrange:
		ConsumerDispatcher dispatcher(Test_Dispatcher_Options, { CreateNoOpConsumer() });
		auto handlerCallCount = 0u;

		// Act:
		auto elementId = dispatcher.processElement(ConsumerInput(model::BlockRange()), [&handlerCallCount](auto, auto) {
			++handlerCallCount;
		});

		// Assert:
		EXPECT_EQ(Test_Dispatcher_Options.DispatcherName, dispatcher.name());
		EXPECT_EQ(1u, dispatcher.size());
		EXPECT_EQ(0u, dispatcher.numAddedElements());
		EXPECT_TRUE(dispatcher.isRunning());
		EXPECT_EQ(0u, elementId);
		EXPECT_EQ(0u, handlerCallCount);
	}

	namespace {
		auto GetExpectedHeights(const std::vector<model::BlockRange>& ranges) {
			return test::Apply(true, ranges, [](const auto& range) {
				return test::Apply(true, range, [](const auto& block) {
					return block.Height;
				});
			});
		}

		auto BlockElementVectorToHeights(BlockElements& entities) {
			return test::Apply(true, entities, [](const auto& blockElement) {
				return blockElement.Block.Height;
			});
		}

		using Heights = std::vector<Height>;

		auto CreateConsumer(std::vector<Heights>& collector) {
			return [&](ConsumerInput& consumerInput) -> ConsumerResult {
				auto heights = BlockElementVectorToHeights(consumerInput.blocks());
				collector.push_back(heights);
				return ConsumerResult::Continue();
			};
		}

		void ProcessAll(ConsumerDispatcher& dispatcher, std::vector<model::BlockRange>&& ranges) {
			for (auto& range : ranges)
				dispatcher.processElement(ConsumerInput(std::move(range)));
		}
	}

	TEST(ConsumerDispatcherTests, ProcessElementReturnsElementId) {
		// Arrange:
		auto ranges = test::PrepareRanges(6);
		ConsumerDispatcher dispatcher(Test_Dispatcher_Options, { CreateNoOpConsumer() });
		std::vector<DisruptorElementId> ids;
		std::vector<DisruptorElementId> expectedIds{ 1, 2, 3, 4, 5, 6 };

		// Act: push single element
		for (auto i = 0u; i < 6; ++i) {
			if (0 == i % 2)
				ids.push_back(dispatcher.processElement(ConsumerInput(std::move(ranges[i]))));
			else
				ids.push_back(dispatcher.processElement(ConsumerInput(std::move(ranges[i])), [](auto, auto) {}));
		}

		// Assert:
		EXPECT_EQ(ranges.size(), dispatcher.numAddedElements());
		EXPECT_EQ(expectedIds, ids);
	}

	TEST(ConsumerDispatcherTests, ConsumerCanConsumeSingleElement) {
		// Arrange:
		auto ranges = test::PrepareRanges(1);
		auto expectedHeights = GetExpectedHeights(ranges);

		std::vector<Heights> collectedHeights;
		ConsumerDispatcher dispatcher(Test_Dispatcher_Options, { CreateConsumer(collectedHeights) });

		// Act: push single element
		ProcessAll(dispatcher, std::move(ranges));
		WAIT_FOR_ONE_EXPR(collectedHeights.size());

		// Assert:
		EXPECT_EQ(1u, collectedHeights.size());
		EXPECT_EQ(ranges.size(), dispatcher.numAddedElements());
		EXPECT_EQ(expectedHeights, collectedHeights);
	}

	TEST(ConsumerDispatcherTests, CanInspectSingleElement) {
		// Arrange:
		auto ranges = test::PrepareRanges(1);
		auto expectedHeights = GetExpectedHeights(ranges);

		std::vector<Heights> collectedHeights;
		auto numInspectorCalls = 0u;
		ConsumerDispatcher dispatcher(
				Test_Dispatcher_Options,
				{ CreateConsumer(collectedHeights) },
				[&numInspectorCalls](const auto&, const auto&) { ++numInspectorCalls; });

		// Act: push single element
		ProcessAll(dispatcher, std::move(ranges));
		WAIT_FOR_ONE_EXPR(numInspectorCalls);
		// - pause is here, to let both the consumer and inspector continue for a bit more,
		//   in case if it would still be running and there'd be bug in implementation
		test::Pause();

		// Assert:
		EXPECT_EQ(1u, collectedHeights.size());
		EXPECT_EQ(ranges.size(), dispatcher.numAddedElements());
		EXPECT_EQ(expectedHeights, collectedHeights);
		EXPECT_EQ(1u, numInspectorCalls);
	}

	TEST(ConsumerDispatcherTests, ConsumerIsNotPassedEmptyInput) {
		// Arrange:
		std::vector<model::BlockRange> ranges;
		ranges.push_back(model::BlockRange());

		auto numConsumerCalls = 0u;
		auto numInspectorCalls = 0u;
		ConsumerDispatcher dispatcher(
				Test_Dispatcher_Options,
				{ [&numConsumerCalls](const auto&) -> ConsumerResult { ++numConsumerCalls; return ConsumerResult::Continue(); } },
				[&numInspectorCalls](const auto&, const auto&) { ++numInspectorCalls; });

		// Act: push single (empty) element
		ProcessAll(dispatcher, std::move(ranges));
		// - pause is here, to let both the consumer and inspector continue for a bit more,
		//   in case if it would still be running and there'd be bug in implementation
		test::Pause();

		// Assert: the element was not passed to the consumer or inspector
		EXPECT_EQ(0u, dispatcher.numAddedElements());
		EXPECT_EQ(0u, numConsumerCalls);
		EXPECT_EQ(0u, numInspectorCalls);
	}

	TEST(ConsumerDispatcherTests, ShutdownStopsDispatcher) {
		// Arrange:
		auto numConsumerCalls = 0u;
		auto numInspectorCalls = 0u;
		auto ranges = test::PrepareRanges(1);
		ConsumerDispatcher dispatcher(
				Test_Dispatcher_Options,
				{ [&numConsumerCalls](const auto&) -> ConsumerResult { ++numConsumerCalls; return ConsumerResult::Continue(); } },
				[&numInspectorCalls](const auto&, const auto&) { ++numInspectorCalls; });

		// Act:
		dispatcher.shutdown();
		ProcessAll(dispatcher, std::move(ranges));

		// Assert:
		EXPECT_EQ(1u, dispatcher.size());
		EXPECT_EQ(ranges.size(), dispatcher.numAddedElements());
		EXPECT_EQ(0u, numConsumerCalls);
		EXPECT_FALSE(dispatcher.isRunning());
		EXPECT_EQ(0u, numInspectorCalls);
	}

	namespace {
		auto CreateCollectingInspector(std::vector<Heights>& heightsCollector, std::vector<CompletionStatus>& statusCollector) {
			return [&](ConsumerInput& input, const ConsumerCompletionResult& completionResult) {
				heightsCollector.push_back(BlockElementVectorToHeights(input.blocks()));
				statusCollector.push_back(completionResult.CompletionStatus);
			};
		}
	}

	TEST(ConsumerDispatcherTests, CanConsumeAndInspectAllElementsWithSingleConsumer) {
		// Arrange:
		auto ranges = test::PrepareRanges(5);
		auto expectedHeights = GetExpectedHeights(ranges);
		std::vector<Heights> collectedHeights;
		std::vector<Heights> inspectedHeights;
		std::vector<CompletionStatus> inspectedStatuses;

		// Act:
		ConsumerDispatcher dispatcher(
				Test_Dispatcher_Options,
				{ CreateConsumer(collectedHeights) },
				CreateCollectingInspector(inspectedHeights, inspectedStatuses));

		// - push multiple elements
		ProcessAll(dispatcher, std::move(ranges));
		WAIT_FOR_VALUE_EXPR(inspectedHeights.size(), 5u);

		// Assert:
		EXPECT_EQ(ranges.size(), dispatcher.numAddedElements());
		EXPECT_EQ(expectedHeights, collectedHeights);
		EXPECT_EQ(expectedHeights, inspectedHeights);
		EXPECT_EQ(std::vector<CompletionStatus>(5, CompletionStatus::Normal), inspectedStatuses);
	}

	TEST(ConsumerDispatcherTests, CanConsumeAndInspectAllElementsWithMultipleConsumers) {
		// Arrange:
		auto ranges = test::PrepareRanges(5);
		auto expectedHeights = GetExpectedHeights(ranges);
		std::vector<Heights> collectedHeights[3];
		std::vector<Heights> inspectedHeights;
		std::vector<CompletionStatus> inspectedStatuses;

		// Act:
		ConsumerDispatcher dispatcher(
				Test_Dispatcher_Options,
				{
					CreateConsumer(collectedHeights[0]),
					CreateConsumer(collectedHeights[1]),
					CreateConsumer(collectedHeights[2]),
				},
				CreateCollectingInspector(inspectedHeights, inspectedStatuses));

		// - push multiple elements
		ProcessAll(dispatcher, std::move(ranges));
		WAIT_FOR_VALUE_EXPR(inspectedHeights.size(), 5u);

		// Assert:
		EXPECT_EQ(ranges.size(), dispatcher.numAddedElements());
		EXPECT_EQ(expectedHeights, collectedHeights[0]);
		EXPECT_EQ(expectedHeights, collectedHeights[1]);
		EXPECT_EQ(expectedHeights, collectedHeights[2]);
		EXPECT_EQ(expectedHeights, inspectedHeights);
		EXPECT_EQ(std::vector<CompletionStatus>(5, CompletionStatus::Normal), inspectedStatuses);
	}

	namespace {
		auto CreateSkipIfFirstBlockIsEvenConsumer() {
			return [](ConsumerInput& consumerInput) -> ConsumerResult {
				return 0 == consumerInput.blocks()[0].Block.Height.unwrap() % 2
						? ConsumerResult::Abort()
						: ConsumerResult::Continue();
			};
		}
	}

	TEST(ConsumerDispatcherTests, MarkedElementsAreSkippedByHigherConsumers) {
		// Arrange:
		std::vector<Heights> collectedHeights;
		auto ranges = test::PrepareRanges(5);
		auto height = 0u;
		for (auto& range : ranges)
			range.begin()->Height = Height(++height);

		auto expectedHeights = test::Filter(GetExpectedHeights(ranges), [](const auto& heights) {
			return 1 == heights[0].unwrap() % 2;
		});

		// Act:
		ConsumerDispatcher dispatcher(
				Test_Dispatcher_Options,
				{
					CreateSkipIfFirstBlockIsEvenConsumer(),
					CreateConsumer(collectedHeights),
				});

		// push multiple elements
		ProcessAll(dispatcher, std::move(ranges));
		WAIT_FOR_VALUE_EXPR(collectedHeights.size(), 3u);

		// Assert:
		EXPECT_EQ(ranges.size(), dispatcher.numAddedElements());
		EXPECT_EQ(expectedHeights, collectedHeights);
	}

	TEST(ConsumerDispatcherTests, MarkedElementsArePassedToInspector) {
		// Arrange:
		std::vector<Heights> inspectedHeights;
		std::vector<CompletionStatus> inspectedStatuses;
		auto ranges = test::PrepareRanges(5);
		auto height = 0u;
		for (auto& range : ranges)
			range.begin()->Height = Height(++height);

		auto expectedHeights = GetExpectedHeights(ranges);

		// Act:
		ConsumerDispatcher dispatcher(
				Test_Dispatcher_Options,
				{ CreateSkipIfFirstBlockIsEvenConsumer() },
				CreateCollectingInspector(inspectedHeights, inspectedStatuses));

		// - push multiple elements
		ProcessAll(dispatcher, std::move(ranges));
		WAIT_FOR_VALUE_EXPR(inspectedHeights.size(), 5u);

		// Assert:
		EXPECT_EQ(ranges.size(), dispatcher.numAddedElements());
		EXPECT_EQ(expectedHeights, inspectedHeights);

		// - ranges have heights 1-5, where even heights should be aborted
		auto expectedStatuses = std::vector<CompletionStatus>(5, CompletionStatus::Normal);
		expectedStatuses[1] = CompletionStatus::Aborted;
		expectedStatuses[3] = CompletionStatus::Aborted;
		EXPECT_EQ(expectedStatuses, inspectedStatuses);
	}

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wused-but-marked-unused"
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4702) /* "unreachable code" */
#endif

	TEST(ConsumerDispatcherTests, ExceptionThrownFromTheConsumerTerminates) {
		ASSERT_DEATH({
			// Arrange:
			auto ranges = test::PrepareRanges(1);

			ConsumerDispatcher dispatcher(
					Test_Dispatcher_Options,
					{
						[](const auto&) -> ConsumerResult {
							CATAPULT_THROW_RUNTIME_ERROR("dummy consumer exception");
							return ConsumerResult::Continue();
						}
					});

			// Act:
			ProcessAll(dispatcher, std::move(ranges));
			WAIT_FOR_VALUE_EXPR(dispatcher.isRunning(), false);
		}, "");
	}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

	TEST(ConsumerDispatcherTests, ConsumerThrowsIfDisruptorSpaceIsExhausted) {
		ASSERT_DEATH({
			// Arrange:
			constexpr size_t Disruptor_Size = 8;
			auto ranges = test::PrepareRanges(Disruptor_Size - 1);
			std::atomic<size_t> counter(0);

			ConsumerDispatcher dispatcher(
					{ "ConsumerDispatcherTests", Disruptor_Size },
					{
						[&](const auto&) -> ConsumerResult {
							++counter;
							CATAPULT_LOG(info) << "consumer is processing element " << counter;
							return ConsumerResult::Continue();
						},
						[&](const auto&) -> ConsumerResult {
							while (true)
								test::Sleep(5);
						}
					});

			// Act: first consumer is processing disruptorSize - 1 elements, second consumer just waits
			ProcessAll(dispatcher, std::move(ranges));
			WAIT_FOR_VALUE(counter, Disruptor_Size - 1);

			// - adding another element to the disruptor will let the first consumer detect the failure
			CATAPULT_LOG(info) << "consumer attempting to process another element";
			auto killerRange = test::CreateBlockEntityRange(1);
			dispatcher.processElement(ConsumerInput(std::move(killerRange)));
			WAIT_FOR_VALUE(counter, Disruptor_Size);
		}, "");
	}

	namespace {
		auto CreateConsumers(
				std::vector<size_t>& counters,
				std::atomic<size_t>& totalCounter,
				size_t offset,
				const test::AutoSetFlag& continueFlag) {
			std::vector<DisruptorConsumer> consumers(counters.size());
			for (auto i = 0u; i < counters.size(); ++i) {
				auto consumer = [&counters, &totalCounter, &continueFlag, i, offset](const auto&) -> ConsumerResult {
					CATAPULT_LOG(info) << "consumer " << i << " is processing element " << (counters[i] + 1);
					++counters[i];
					++totalCounter;
					if (counters[i] < counters.size() - i + offset - 1)
						return ConsumerResult::Continue();

					CATAPULT_LOG(info) << "consumer " << i << " reached blocking position";
					while (!continueFlag.isSet())
						test::Sleep(5);

					return ConsumerResult::Continue();
				};
				consumers[i] = std::move(consumer);
			}

			return consumers;
		}

		void AssertDeathIfDisruptorSpaceIsExhaustedEvenIfOtherConsumersHaveNearbyPositions(size_t offset) {
			ASSERT_DEATH({
				// Arrange:
				std::atomic<size_t> totalCounter(0);
				constexpr auto Disruptor_Size = 4u;
				std::vector<size_t> counters(Disruptor_Size);
				std::unique_ptr<ConsumerDispatcher> pDispatcher;
				test::AutoSetFlag continueFlag;
				auto consumers = CreateConsumers(counters, totalCounter, offset, continueFlag);
				pDispatcher = std::make_unique<ConsumerDispatcher>(
						ConsumerDispatcherOptions({ "ConsumerDispatcherTests", Disruptor_Size }),
						consumers);

				// - let consumers process offset ranges, their position is at offset afterwards
				//   (except for the inspector which is at position offset - 1)
				auto phase1Ranges = test::PrepareRanges(offset);
				for (auto i = 0u; i < offset; ++i) {
					pDispatcher->processElement(ConsumerInput(std::move(phase1Ranges[i])));

					// wait for each element to be fully processed so consumer 0 does not get too far ahead
					WAIT_FOR_VALUE(totalCounter, (i + 1) * Disruptor_Size);
				}

				// Act:
				// - consumer 0 + 1 will advance Disruptor_Size - 2 positions
				// - consumer 2 will advance Disruptor_Size - 3 positions
				// - consumer 3 will stay where it is
				auto phase2Ranges = test::PrepareRanges(Disruptor_Size - 2);
				ProcessAll(*pDispatcher, std::move(phase2Ranges));
				WAIT_FOR_VALUE(totalCounter, offset * Disruptor_Size + 2 + 2 + 1);

				CATAPULT_LOG(info) << "all consumers reached their predefined positions";
				for (auto i = 0u; i < counters.size(); ++i)
					CATAPULT_LOG(debug) << "counters[" << i << "]: " << counters[i];

				// adding another element to the disruptor will let consumer 0 detect the failure
				// consumer 0 will attempt to process at (offset - 1 + Disruptor_Size) but inspector is only at (offset - 1)
				auto killerRange = test::CreateBlockEntityRange(1);
				CATAPULT_LOG(info) << "consumer 0 is processing the deadly element";
				pDispatcher->processElement(ConsumerInput(std::move(killerRange)));

				// intentionally use dummy value, as this code should never be reached (due to earlier throw)
				WAIT_FOR_VALUE_EXPR(counters[0], 123456u);
			}, "");
		}
	}

	TEST(ConsumerDispatcherTests, ConsumerThrowsIfDisruptorSpaceIsExhaustedEvenIfOtherConsumersHaveNearbyPositions_NoWrapAround) {
		AssertDeathIfDisruptorSpaceIsExhaustedEvenIfOtherConsumersHaveNearbyPositions(1);
	}

	TEST(ConsumerDispatcherTests, ConsumerThrowsIfDisruptorSpaceIsExhaustedEvenIfOtherConsumersHaveNearbyPositions_WrapAround) {
		AssertDeathIfDisruptorSpaceIsExhaustedEvenIfOtherConsumersHaveNearbyPositions(10);
	}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
}}

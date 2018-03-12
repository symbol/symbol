#include "catapult/disruptor/DisruptorConsumer.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/other/DisruptorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace disruptor {

#define TEST_CLASS DisruptorConsumerTests

	namespace {
		static auto CreateConsumerInputWithBlocks(size_t numBlocks) {
			return ConsumerInput(test::CreateBlockEntityRange(numBlocks));
		}

		static auto CreateConsumerInputWithTransactions(size_t numTransactions) {
			return ConsumerInput(test::CreateTransactionEntityRange(numTransactions));
		}

		struct BlockTraits {
			using ConsumersType = std::vector<BlockConsumer>;

			static constexpr auto FromTypedConsumers = DisruptorConsumersFromBlockConsumers;
			static constexpr auto CreateInput = CreateConsumerInputWithBlocks;
			static constexpr auto CreateInvalidInput = CreateConsumerInputWithTransactions;

			static auto GetDataPointer(const ConsumerInput& input) {
				return input.blocks().data();
			}
		};

		struct TransactionTraits {
			using ConsumersType = std::vector<TransactionConsumer>;

			static constexpr auto FromTypedConsumers = DisruptorConsumersFromTransactionConsumers;
			static constexpr auto CreateInput = CreateConsumerInputWithTransactions;
			static constexpr auto CreateInvalidInput = CreateConsumerInputWithBlocks;

			static auto GetDataPointer(const ConsumerInput& input) {
				return input.transactions().data();
			}
		};

#define ENTITY_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Block) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BlockTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Transaction) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransactionTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()
	}

	ENTITY_TRAITS_BASED_TEST(FromTypedConsumers_CannotMapZeroConsumers) {
		// Arrange:
		typename TTraits::ConsumersType typedConsumers;

		// Act + Assert:
		EXPECT_THROW(TTraits::FromTypedConsumers(typedConsumers), catapult_invalid_argument);
	}

	namespace {
		template<typename TTraits>
		void AssertInvalidInput(ConsumerInput&& input) {
			// Arrange: create a single consumer
			auto numCalls = 0u;
			typename TTraits::ConsumersType typedConsumers{
				[&numCalls](const auto&) {
					++numCalls;
					return ConsumerResult::Continue();
				}
			};

			// Act: perform the mapping
			auto consumers = TTraits::FromTypedConsumers(typedConsumers);
			ASSERT_EQ(1u, consumers.size());

			// - invoke with an invalid input
			EXPECT_THROW(consumers[0](input), catapult_runtime_error);

			// Assert: the wrapped consumer was not called
			EXPECT_EQ(0u, numCalls);
		}
	}

	ENTITY_TRAITS_BASED_TEST(FromTypedConsumers_FailsForEmptyInput) {
		// Assert: empty input has no matching elements
		AssertInvalidInput<TTraits>(ConsumerInput());
	}

	ENTITY_TRAITS_BASED_TEST(FromTypedConsumers_FailsForInvalidInput) {
		// Assert: invalid input has no matching elements
		AssertInvalidInput<TTraits>(TTraits::CreateInvalidInput(2));
	}

	ENTITY_TRAITS_BASED_TEST(FromTypedConsumers_DelegatesToTypedConsumerForValidInput) {
		// Arrange: create a single consumer
		auto numCalls = 0u;
		const void* pElementData = nullptr;
		typename TTraits::ConsumersType typedConsumers{
			[&numCalls, &pElementData](const auto& elements) {
				++numCalls;
				pElementData = elements.data();
				return ConsumerResult::Continue();
			}
		};

		// Act: perform the mapping
		auto consumers = TTraits::FromTypedConsumers(typedConsumers);
		ASSERT_EQ(1u, consumers.size());

		// - invoke with a valid input
		auto input = TTraits::CreateInput(2);
		auto result = consumers[0](input);

		// Assert: the result is continue and the wrapped consumer was called once with the expected input
		test::AssertContinued(result);
		EXPECT_EQ(1u, numCalls);
		EXPECT_EQ(TTraits::GetDataPointer(input), pElementData);
	}

	ENTITY_TRAITS_BASED_TEST(FromTypedConsumers_TypedConsumerCanModifyInput) {
		// Arrange: create a single consumer
		auto hash = test::GenerateRandomData<Hash256_Size>();
		typename TTraits::ConsumersType typedConsumers{
			[&hash](auto& elements) {
				elements[0].EntityHash = hash;
				return ConsumerResult::Continue();
			}
		};

		// Act: perform the mapping
		auto consumers = TTraits::FromTypedConsumers(typedConsumers);
		ASSERT_EQ(1u, consumers.size());

		// - invoke with a valid input
		auto input = TTraits::CreateInput(2);
		auto result = consumers[0](input);

		// Assert: input hash was changed
		test::AssertContinued(result);
		EXPECT_EQ(hash, TTraits::GetDataPointer(input)->EntityHash);
	}

	ENTITY_TRAITS_BASED_TEST(FromTypedConsumers_CanMapMultipleConsumers) {
		// Arrange: create multiple consumers
		constexpr auto Num_Consumers = 4u;
		std::vector<size_t> consumerCallCounts(Num_Consumers);
		typename TTraits::ConsumersType typedConsumers;
		for (auto i = 0u; i < Num_Consumers; ++i) {
			consumerCallCounts[i] = 0;
			typedConsumers.push_back([&consumerCallCounts, i](const auto&) {
				++consumerCallCounts[i];
				return 0 == i % 2 ? ConsumerResult::Continue() : ConsumerResult::Abort(i * 2);
			});
		}

		// Act: perform the mapping
		auto consumers = TTraits::FromTypedConsumers(typedConsumers);
		ASSERT_EQ(Num_Consumers, consumers.size());

		// - invoke with a valid input
		auto i = 0u;
		auto input = TTraits::CreateInput(2);
		for (const auto& consumer : consumers) {
			auto result = consumer(input);

			// Assert: the result is correct and the wrapped consumer was called once
			if (0 == i % 2)
				test::AssertContinued(result);
			else
				test::AssertAborted(result, i * 2);

			EXPECT_EQ(1u, consumerCallCounts[i]);
			++i;
		}
	}
}}

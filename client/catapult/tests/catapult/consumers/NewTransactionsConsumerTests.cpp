#include "catapult/consumers/TransactionConsumers.h"
#include "tests/catapult/consumers/utils/ConsumerInputFactory.h"
#include "tests/catapult/consumers/utils/ConsumerTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/nodeps/ParamsCapture.h"
#include "tests/TestHarness.h"

using catapult::disruptor::ConsumerInput;

namespace catapult { namespace consumers {

	namespace {
		struct NewTransactionsSinkParams {
		public:
			explicit NewTransactionsSinkParams(std::vector<model::TransactionInfo>&& addedTransactionInfos)
					: AddedTransactionInfos(CopyInfos(addedTransactionInfos))
			{}

		private:
			static std::vector<model::TransactionInfo> CopyInfos(const std::vector<model::TransactionInfo>& original) {
				std::vector<model::TransactionInfo> copy;
				copy.reserve(original.size());
				for (const auto& info : original)
					copy.emplace_back(info.copy());

				return copy;
			}

		public:
			std::vector<model::TransactionInfo> AddedTransactionInfos;
		};

		class MockNewTransactionsSink : public test::ParamsCapture<NewTransactionsSinkParams> {
		public:
			void operator()(std::vector<model::TransactionInfo>&& addedTransactionInfos) const {
				const_cast<MockNewTransactionsSink*>(this)->push(std::move(addedTransactionInfos));
			}
		};

		struct ConsumerTestContext {
		public:
			ConsumerTestContext()
					: Consumer(CreateNewTransactionsConsumer([&handler = NewTransactionsSink](auto&& infos) {
						handler(std::move(infos));
					}))
			{}

		public:
			MockNewTransactionsSink NewTransactionsSink;
			disruptor::DisruptorConsumer Consumer;
		};

		ConsumerInput CreateInput(size_t numTransactions) {
			return test::CreateConsumerInputWithTransactions(numTransactions, disruptor::InputSource::Unknown);
		}

		void AssertEqual(
				const disruptor::FreeTransactionElement& element,
				const model::TransactionInfo& info,
				const std::string& message) {
			// Assert:
			EXPECT_EQ(&element.Transaction, info.pEntity.get()) << message;
			EXPECT_EQ(element.EntityHash, info.EntityHash) << message;
			EXPECT_EQ(element.MerkleComponentHash, info.MerkleComponentHash) << message;

			// Sanity:
			EXPECT_FALSE(element.Skip) << message;
		}
	}

	TEST(NewTransactionsConsumerTests, CanProcessZeroEntities) {
		// Assert:
		ConsumerTestContext context;
		test::AssertPassthroughForEmptyInput(context.Consumer);
	}

	TEST(NewTransactionsConsumerTests, AllEntitiesAreForwardedWhenNoneAreSkipped) {
		// Arrange:
		ConsumerTestContext context;
		auto input = CreateInput(3);

		// Act:
		auto result = context.Consumer(input);

		// Assert: the consumer detached the input
		test::AssertConsumed(result);
		EXPECT_TRUE(input.empty());

		// - the new transactions handler was called once (with three entities)
		const auto& params = context.NewTransactionsSink.params();
		ASSERT_EQ(1u, params.size());
		const auto& actualInfos = params[0].AddedTransactionInfos;
		EXPECT_EQ(3u, actualInfos.size());

		auto i = 0u;
		for (const auto& info : actualInfos) {
			AssertEqual(input.transactions()[i], info, "info at " + std::to_string(i));
			++i;
		}
	}

	TEST(NewTransactionsConsumerTests, NoEntitiesAreForwardedWhenAllAreSkipped) {
		// Arrange:
		ConsumerTestContext context;
		auto input = CreateInput(3);
		for (auto& element : input.transactions())
			element.Skip = true;

		// Act:
		auto result = context.Consumer(input);

		// Assert: the consumer detached the input
		test::AssertConsumed(result);
		EXPECT_TRUE(input.empty());

		// - the new transactions handler was called once (with zero entities)
		const auto& params = context.NewTransactionsSink.params();
		ASSERT_EQ(1u, params.size());
		const auto& actualInfos = params[0].AddedTransactionInfos;
		EXPECT_TRUE(actualInfos.empty());
	}

	TEST(NewTransactionsConsumerTests, OnlyNonSkippedElementsAreForwardedWhenSomeAreSkipped) {
		// Arrange:
		ConsumerTestContext context;
		auto input = CreateInput(5);
		for (auto i : { 0u, 2u, 3u })
			input.transactions()[i].Skip = true;

		// Act:
		auto result = context.Consumer(input);

		// Assert: the consumer detached the input
		test::AssertConsumed(result);
		EXPECT_TRUE(input.empty());

		// - the new transactions handler was called once (with two entities)
		const auto& params = context.NewTransactionsSink.params();
		ASSERT_EQ(1u, params.size());
		const auto& actualInfos = params[0].AddedTransactionInfos;
		EXPECT_EQ(2u, actualInfos.size());

		AssertEqual(input.transactions()[1], actualInfos[0], "info at 0");
		AssertEqual(input.transactions()[4], actualInfos[1], "info at 1");
	}
}}

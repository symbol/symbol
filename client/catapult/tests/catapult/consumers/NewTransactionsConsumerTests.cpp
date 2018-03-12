#include "catapult/consumers/TransactionConsumers.h"
#include "tests/catapult/consumers/test/ConsumerInputFactory.h"
#include "tests/catapult/consumers/test/ConsumerTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/nodeps/ParamsCapture.h"
#include "tests/TestHarness.h"

using catapult::disruptor::ConsumerInput;

namespace catapult { namespace consumers {

#define TEST_CLASS NewTransactionsConsumerTests

	namespace {
		struct NewTransactionsSinkParams {
		public:
			explicit NewTransactionsSinkParams(std::vector<model::TransactionInfo>&& addedTransactionInfos)
					: AddedTransactionInfos(CopyInfos(addedTransactionInfos))
			{}

		private:
			static std::vector<model::TransactionInfo> CopyInfos(const std::vector<model::TransactionInfo>& transactionInfos) {
				std::vector<model::TransactionInfo> copy;
				copy.reserve(transactionInfos.size());
				for (const auto& transactionInfo : transactionInfos)
					copy.emplace_back(transactionInfo.copy());

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
					: Consumer(CreateNewTransactionsConsumer([&handler = NewTransactionsSink](auto&& transactionInfos) {
						handler(std::move(transactionInfos));
					}))
			{}

		public:
			MockNewTransactionsSink NewTransactionsSink;
			disruptor::DisruptorConsumer Consumer;
		};

		ConsumerInput CreateInput(size_t numTransactions) {
			auto input = test::CreateConsumerInputWithTransactions(numTransactions, disruptor::InputSource::Unknown);
			for (auto& element : input.transactions())
				element.OptionalExtractedAddresses = std::make_shared<model::AddressSet>();

			return input;
		}

		void AssertEqual(
				const disruptor::FreeTransactionElement& element,
				const model::TransactionInfo& transactionInfo,
				const std::string& message) {
			// Assert:
			EXPECT_EQ(&element.Transaction, transactionInfo.pEntity.get()) << message;
			EXPECT_EQ(element.EntityHash, transactionInfo.EntityHash) << message;
			EXPECT_EQ(element.MerkleComponentHash, transactionInfo.MerkleComponentHash) << message;

			EXPECT_TRUE(!!element.OptionalExtractedAddresses) << message;
			EXPECT_EQ(element.OptionalExtractedAddresses.get(), transactionInfo.OptionalExtractedAddresses.get()) << message;

			// Sanity:
			EXPECT_FALSE(element.Skip) << message;
		}
	}

	TEST(TEST_CLASS, CanProcessZeroEntities) {
		// Assert:
		ConsumerTestContext context;
		test::AssertPassthroughForEmptyInput(context.Consumer);
	}

	TEST(TEST_CLASS, AllEntitiesAreForwardedWhenNoneAreSkipped) {
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
		ASSERT_EQ(3u, actualInfos.size());

		auto i = 0u;
		for (const auto& info : actualInfos) {
			AssertEqual(input.transactions()[i], info, "info at " + std::to_string(i));
			++i;
		}
	}

	TEST(TEST_CLASS, NoEntitiesAreForwardedWhenAllAreSkipped) {
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

	TEST(TEST_CLASS, OnlyNonSkippedElementsAreForwardedWhenSomeAreSkipped) {
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
		ASSERT_EQ(2u, actualInfos.size());

		AssertEqual(input.transactions()[1], actualInfos[0], "info at 0");
		AssertEqual(input.transactions()[4], actualInfos[1], "info at 1");
	}
}}

/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#include "catapult/consumers/BlockConsumers.h"
#include "catapult/consumers/InputUtils.h"
#include "catapult/consumers/TransactionConsumers.h"
#include "catapult/model/TransactionStatus.h"
#include "tests/catapult/consumers/test/ConsumerTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/nodeps/ParamsCapture.h"
#include "tests/TestHarness.h"

using namespace catapult::validators;

namespace catapult { namespace consumers {

#define BLOCK_TEST_CLASS BlockStatelessValidationConsumerTests
#define TRANSACTION_TEST_CLASS TransactionStatelessValidationConsumerTests

	namespace {
		// region MockParallelValidationPolicy

		struct DispatchParams {
		public:
			explicit DispatchParams(const model::WeakEntityInfos& entityInfos) : EntityInfos(entityInfos){
				for (const auto& entityInfo : EntityInfos)
					HashCopies.push_back(entityInfo.hash());
			}

		public:
			model::WeakEntityInfos EntityInfos;
			std::vector<Hash256> HashCopies;
		};

		template<typename TResultType>
		class BasicMockParallelValidationPolicy
				: public test::ParamsCapture<DispatchParams>
				, public ParallelValidationPolicy {
		protected:
			using ResultType = TResultType;

		public:
			BasicMockParallelValidationPolicy()
					: m_numValidateCalls(0)
					, m_validateTrigger(std::numeric_limits<size_t>::max())
			{}

		public:
			void setResult(const ResultType& result, size_t trigger = 0) {
				m_result = result;
				m_validateTrigger = trigger;
			}

		public:
			[[noreturn]]
			thread::future<ValidationResult> validateShortCircuit(const model::WeakEntityInfos&) const override {
				CATAPULT_THROW_RUNTIME_ERROR("validateShortCircuit not implemented");
			}

			[[noreturn]]
			thread::future<std::vector<ValidationResult>> validateAll(const model::WeakEntityInfos&) const override {
				CATAPULT_THROW_RUNTIME_ERROR("validateAll not implemented");
			}

		protected:
			thread::future<ResultType> validateImpl(const model::WeakEntityInfos& entityInfos, const ResultType& defaultResult) const {
				const_cast<BasicMockParallelValidationPolicy*>(this)->push(entityInfos);

				// determine the result based on the call count
				auto result = ++m_numValidateCalls < m_validateTrigger ? defaultResult : m_result;
				return toResult(result);
			}

		private:
			thread::future<ResultType> toResult(ResultType& result) const {
				thread::promise<ResultType> promise;
				promise.set_value(std::move(result));
				return promise.get_future();
			}

		private:
			ResultType m_result;
			mutable size_t m_numValidateCalls;
			size_t m_validateTrigger;
		};

		class MockParallelShortCircuitValidationPolicy : public BasicMockParallelValidationPolicy<ValidationResult> {
		public:
			thread::future<ResultType> validateShortCircuit(const model::WeakEntityInfos& entityInfos) const override {
				return validateImpl(entityInfos, ValidationResult::Success);
			}
		};

		class MockParallelAllValidationPolicy : public BasicMockParallelValidationPolicy<std::vector<ValidationResult>> {
		public:
			thread::future<ResultType> validateAll(const model::WeakEntityInfos& entityInfos) const override {
				return validateImpl(entityInfos, ResultType(entityInfos.size(), ValidationResult::Success));
			}
		};

		// endregion
	}

	// region block - utils + traits

	namespace {
		constexpr bool RequiresAllPredicate(model::BasicEntityType, Timestamp, const Hash256&) {
			return true;
		}

		struct BlockTestContext {
		public:
			explicit BlockTestContext(const RequiresValidationPredicate& requiresValidationPredicate = RequiresAllPredicate)
					: pPolicy(std::make_shared<MockParallelShortCircuitValidationPolicy>())
					, Consumer(CreateBlockStatelessValidationConsumer(pPolicy, requiresValidationPredicate))
			{}

		public:
			std::shared_ptr<MockParallelShortCircuitValidationPolicy> pPolicy;
			disruptor::ConstBlockConsumer Consumer;
		};

		struct BlockTraits {
			using TestContextType = BlockTestContext;

			static constexpr auto Num_Sub_Entities_Single = 4u;
			static constexpr auto Num_Sub_Entities_Multiple = 10u;

			static auto CreateSingleEntityElements() {
				auto pBlock = test::GenerateBlockWithTransactions(3, Height(246));
				return test::CreateBlockElements({ pBlock.get() });
			}

			static auto CreateMultipleEntityElements() {
				auto pBlock1 = test::GenerateBlockWithTransactions(1, Height(246));
				auto pBlock2 = test::GenerateBlockWithTransactions(0, Height(247));
				auto pBlock3 = test::GenerateBlockWithTransactions(3, Height(248));
				auto pBlock4 = test::GenerateBlockWithTransactions(2, Height(249));
				return test::CreateBlockElements({ pBlock1.get(), pBlock2.get(), pBlock3.get(), pBlock4.get() });
			}

			static void AssertEntities(
					const disruptor::BlockElements& elements,
					const std::vector<DispatchParams>& params,
					size_t numExpectedEntities,
					const RequiresValidationPredicate& requiresValidationPredicate = RequiresAllPredicate) {
				// Arrange:
				model::WeakEntityInfos expectedEntityInfos;
				ExtractMatchingEntityInfos(elements, expectedEntityInfos, requiresValidationPredicate);

				// Assert:
				ASSERT_EQ(1u, params.size());
				EXPECT_EQ(numExpectedEntities, params[0].EntityInfos.size());
				EXPECT_EQ(expectedEntityInfos.size(), params[0].EntityInfos.size());
				EXPECT_EQ(expectedEntityInfos, params[0].EntityInfos);
			}
		};
	}

	// endregion

	// region block

	TEST(BLOCK_TEST_CLASS, CanProcessZeroEntities) {
		// Arrange:
		BlockTraits::TestContextType context;

		// Assert:
		test::AssertPassthroughForEmptyInput(context.Consumer);
	}

	TEST(BLOCK_TEST_CLASS, CanValidateSingleEntity) {
		// Arrange:
		BlockTraits::TestContextType context;
		auto elements = BlockTraits::CreateSingleEntityElements();

		// Act:
		auto result = context.Consumer(elements);

		// Assert:
		test::AssertContinued(result);
		BlockTraits::AssertEntities(elements, context.pPolicy->params(), BlockTraits::Num_Sub_Entities_Single);
	}

	TEST(BLOCK_TEST_CLASS, CanValidateMultipleEntities) {
		// Arrange:
		BlockTraits::TestContextType context;
		auto elements = BlockTraits::CreateMultipleEntityElements();

		// Act:
		auto result = context.Consumer(elements);

		// Assert:
		test::AssertContinued(result);
		BlockTraits::AssertEntities(elements, context.pPolicy->params(), BlockTraits::Num_Sub_Entities_Multiple);
	}

	namespace {
		void AssertBlockAbortResult(ValidationResult validationResult) {
			// Arrange:
			BlockTraits::TestContextType context;
			auto elements = BlockTraits::CreateSingleEntityElements();

			context.pPolicy->setResult(validationResult);

			// Act:
			auto result = context.Consumer(elements);

			// Assert:
			test::AssertAborted(result, validationResult, disruptor::ConsumerResultSeverity::Fatal);
			BlockTraits::AssertEntities(elements, context.pPolicy->params(), BlockTraits::Num_Sub_Entities_Single);
		}
	}

	TEST(BLOCK_TEST_CLASS, NeutralValidationResultIsMappedToAbortConsumerResult) {
		AssertBlockAbortResult(ValidationResult::Neutral);
	}

	TEST(BLOCK_TEST_CLASS, FailureValidationResultIsMappedToAbortConsumerResult) {
		AssertBlockAbortResult(ValidationResult::Failure);
	}

	TEST(BLOCK_TEST_CLASS, CanValidateEmptyBlock) {
		// Arrange:
		BlockTestContext context;
		auto pBlock = test::GenerateBlockWithTransactions(0, Height(246));
		auto elements = test::CreateBlockElements({ pBlock.get() });

		// Act:
		auto result = context.Consumer(elements);

		// Assert:
		test::AssertContinued(result);
		BlockTraits::AssertEntities(elements, context.pPolicy->params(), 1);
	}

	TEST(BLOCK_TEST_CLASS, CanValidateMultipleBlocksWithCustomPredicate) {
		// Arrange:
		auto predicateFactory = []() {
			auto pCounter = std::make_shared<size_t>(0);
			return [pCounter](auto, auto, const auto&) { return 0 == (*pCounter)++ % 2; };
		};

		BlockTestContext context(predicateFactory());
		auto elements = BlockTraits::CreateMultipleEntityElements();

		// Act:
		auto result = context.Consumer(elements);

		// Assert:
		test::AssertContinued(result);

		auto numExpectedEntities = BlockTraits::Num_Sub_Entities_Multiple / 2;
		BlockTraits::AssertEntities(elements, context.pPolicy->params(), numExpectedEntities, predicateFactory());
	}

	// endregion

	// region transaction - utils + traits

	namespace {
		struct TransactionTestContext {
		public:
			TransactionTestContext()
					: pPolicy(std::make_shared<MockParallelAllValidationPolicy>())
					, Consumer(CreateTransactionStatelessValidationConsumer(pPolicy, [this](
							const auto& transaction,
							const auto& hash,
							auto result) {
						// notice that transaction.Deadline is used as transaction marker
						FailedTransactionStatuses.emplace_back(hash, transaction.Deadline, utils::to_underlying_type(result));
					}))
			{}

		public:
			std::shared_ptr<MockParallelAllValidationPolicy> pPolicy;
			std::vector<model::TransactionStatus> FailedTransactionStatuses;
			disruptor::TransactionConsumer Consumer;
		};

		struct TransactionTraits {
			using TestContextType = TransactionTestContext;

			static auto CreateSingleEntityElements() {
				auto pTransaction = test::GenerateRandomTransaction();
				return test::CreateTransactionElements({ pTransaction.get() });
			}

			static auto CreateMultipleEntityElements() {
				auto pTransaction1 = test::GenerateRandomTransaction();
				auto pTransaction2 = test::GenerateRandomTransaction();
				auto pTransaction3 = test::GenerateRandomTransaction();
				auto pTransaction4 = test::GenerateRandomTransaction();
				return test::CreateTransactionElements({
					pTransaction1.get(), pTransaction2.get(), pTransaction3.get(), pTransaction4.get()
				});
			}

			static void AssertEntities(const model::WeakEntityInfos& expectedEntityInfos, const std::vector<DispatchParams>& params) {
				// Assert:
				ASSERT_EQ(1u, params.size());
				EXPECT_EQ(expectedEntityInfos.size(), params[0].EntityInfos.size());
				EXPECT_EQ(expectedEntityInfos, params[0].EntityInfos);
			}
		};

		model::WeakEntityInfos FilterEntityInfos(const TransactionElements& elements, const std::set<size_t>& indexes) {
			auto index = 0u;
			model::WeakEntityInfos entityInfos;
			for (const auto& element : elements) {
				if (indexes.cend() != indexes.find(index++))
					entityInfos.emplace_back(element.Transaction, element.EntityHash);
			}

			return entityInfos;
		}

		void AssertSkipped(
				const TransactionElements& elements,
				const std::set<size_t>& skippedIndexes,
				const std::vector<disruptor::ConsumerResultSeverity>& expectedNonSuccessSeverities) {
			// Sanity:
			ASSERT_EQ(skippedIndexes.size(), expectedNonSuccessSeverities.size());

			auto index = 0u;
			auto numUsedIndexes = 0u;
			for (const auto& element : elements) {
				auto expectedResultCode = skippedIndexes.cend() != skippedIndexes.find(index)
						? expectedNonSuccessSeverities[numUsedIndexes++]
						: disruptor::ConsumerResultSeverity::Success;

				EXPECT_EQ(expectedResultCode, element.ResultSeverity);
				++index;
			}
		}
	}

	// endregion

	// region transaction

	TEST(TRANSACTION_TEST_CLASS, CanProcessZeroEntities) {
		// Arrange:
		TransactionTraits::TestContextType context;

		// Assert:
		test::AssertPassthroughForEmptyInput(context.Consumer);
	}

	TEST(TRANSACTION_TEST_CLASS, CanValidateSingleEntity) {
		// Arrange:
		TransactionTraits::TestContextType context;
		auto elements = TransactionTraits::CreateSingleEntityElements();

		// Act:
		auto result = context.Consumer(elements);

		// Assert:
		test::AssertContinued(result);
		TransactionTraits::AssertEntities(FilterEntityInfos(elements, { 0 }), context.pPolicy->params());
		AssertSkipped(elements, {}, {});
		EXPECT_TRUE(context.FailedTransactionStatuses.empty());
	}

	TEST(TRANSACTION_TEST_CLASS, CanValidateMultipleEntities) {
		// Arrange:
		TransactionTraits::TestContextType context;
		auto elements = TransactionTraits::CreateMultipleEntityElements();

		// Act:
		auto result = context.Consumer(elements);

		// Assert:
		test::AssertContinued(result);
		TransactionTraits::AssertEntities(FilterEntityInfos(elements, { 0, 1, 2, 3 }), context.pPolicy->params());
		AssertSkipped(elements, {}, {});
		EXPECT_TRUE(context.FailedTransactionStatuses.empty());
	}

	// endregion

	// region transaction - neutral results

	TEST(TRANSACTION_TEST_CLASS, NeutralValidationResultIsMappedToAbortConsumerResult) {
		// Arrange:
		TransactionTraits::TestContextType context;
		auto elements = TransactionTraits::CreateSingleEntityElements();

		context.pPolicy->setResult({ ValidationResult::Neutral });

		// Act:
		auto result = context.Consumer(elements);

		// Assert:
		test::AssertAborted(result, ValidationResult::Neutral, disruptor::ConsumerResultSeverity::Fatal);
		TransactionTraits::AssertEntities(FilterEntityInfos(elements, { 0 }), context.pPolicy->params());
		AssertSkipped(elements, { 0 }, { disruptor::ConsumerResultSeverity::Neutral });
		EXPECT_TRUE(context.FailedTransactionStatuses.empty());
	}

	TEST(TRANSACTION_TEST_CLASS, PartialNeutralValidationResultIsMappedToAbortConsumerResult) {
		// Arrange:
		TransactionTraits::TestContextType context;
		auto elements = TransactionTraits::CreateMultipleEntityElements();

		// - configure some elements to be validated as success and others as neutral
		constexpr auto Neutral_Result = ValidationResult::Neutral;
		context.pPolicy->setResult({ ValidationResult::Success, Neutral_Result, Neutral_Result, ValidationResult::Success });

		// Act:
		auto result = context.Consumer(elements);

		// Assert:
		test::AssertAborted(result, ValidationResult::Neutral, disruptor::ConsumerResultSeverity::Fatal);
		TransactionTraits::AssertEntities(FilterEntityInfos(elements, { 0, 1, 2, 3 }), context.pPolicy->params());
		AssertSkipped(elements, { 1, 2 }, { disruptor::ConsumerResultSeverity::Neutral, disruptor::ConsumerResultSeverity::Neutral });
		EXPECT_TRUE(context.FailedTransactionStatuses.empty());
	}

	// endregion

	// region transaction - failure results

#define EXPECT_EQ_STATUS(EXPECTED_ELEMENT, EXPECTED_RESULT, STATUS) \
	do { \
		EXPECT_EQ(EXPECTED_ELEMENT.EntityHash, STATUS.Hash); \
		EXPECT_EQ(utils::to_underlying_type(EXPECTED_RESULT), STATUS.Status); \
		EXPECT_EQ(EXPECTED_ELEMENT.Transaction.Deadline, STATUS.Deadline); \
	} while (false)

	TEST(TRANSACTION_TEST_CLASS, FailureValidationResultIsMappedToAbortConsumerResult) {
		// Arrange:
		TransactionTraits::TestContextType context;
		auto elements = TransactionTraits::CreateSingleEntityElements();

		context.pPolicy->setResult({ ValidationResult::Failure });

		// Act:
		auto result = context.Consumer(elements);

		// Assert:
		test::AssertAborted(result, ValidationResult::Failure, disruptor::ConsumerResultSeverity::Fatal);
		TransactionTraits::AssertEntities(FilterEntityInfos(elements, { 0 }), context.pPolicy->params());
		AssertSkipped(elements, { 0 }, { disruptor::ConsumerResultSeverity::Failure });

		ASSERT_EQ(1u, context.FailedTransactionStatuses.size());
		EXPECT_EQ_STATUS(elements[0], ValidationResult::Failure, context.FailedTransactionStatuses[0]);
	}

	TEST(TRANSACTION_TEST_CLASS, PartialFailureValidationResultIsMappedToAbortConsumerResult) {
		// Arrange:
		TransactionTraits::TestContextType context;
		auto elements = TransactionTraits::CreateMultipleEntityElements();

		// - configure some elements to be validated as success and others as failure
		constexpr auto Failure_Result2 = MakeValidationResult(ResultSeverity::Failure, FacilityCode::Core, 0, ResultFlags::None);
		context.pPolicy->setResult({ ValidationResult::Success, ValidationResult::Failure, Failure_Result2, ValidationResult::Success });

		// Act:
		auto result = context.Consumer(elements);

		// Assert:
		test::AssertAborted(result, ValidationResult::Failure, disruptor::ConsumerResultSeverity::Fatal);
		TransactionTraits::AssertEntities(FilterEntityInfos(elements, { 0, 1, 2, 3 }), context.pPolicy->params());
		AssertSkipped(elements, { 1, 2 }, { disruptor::ConsumerResultSeverity::Failure, disruptor::ConsumerResultSeverity::Failure });

		ASSERT_EQ(2u, context.FailedTransactionStatuses.size());
		EXPECT_EQ_STATUS(elements[1], ValidationResult::Failure, context.FailedTransactionStatuses[0]);
		EXPECT_EQ_STATUS(elements[2], Failure_Result2, context.FailedTransactionStatuses[1]);
	}

	TEST(TRANSACTION_TEST_CLASS, AllNonSuccessValidationResultIsMappedToAbortConsumerResult) {
		// Arrange:
		TransactionTraits::TestContextType context;
		auto elements = TransactionTraits::CreateMultipleEntityElements();

		// - configure some elements to be validated as neutral and others as failure
		constexpr auto Failure_Result1 = MakeValidationResult(ResultSeverity::Failure, FacilityCode::Core, 0, ResultFlags::None);
		context.pPolicy->setResult({ ValidationResult::Neutral, Failure_Result1, ValidationResult::Failure, ValidationResult::Neutral });

		// Act:
		auto result = context.Consumer(elements);

		// Assert: notice that the first failure result is used (basic ValidationResult aggregation)
		test::AssertAborted(result, Failure_Result1, disruptor::ConsumerResultSeverity::Fatal);
		TransactionTraits::AssertEntities(FilterEntityInfos(elements, { 0, 1, 2, 3 }), context.pPolicy->params());
		AssertSkipped(elements, { 0, 1, 2, 3 }, {
			disruptor::ConsumerResultSeverity::Neutral,
			disruptor::ConsumerResultSeverity::Failure,
			disruptor::ConsumerResultSeverity::Failure,
			disruptor::ConsumerResultSeverity::Neutral
		});

		ASSERT_EQ(2u, context.FailedTransactionStatuses.size());
		EXPECT_EQ_STATUS(elements[1], Failure_Result1, context.FailedTransactionStatuses[0]);
		EXPECT_EQ_STATUS(elements[2], ValidationResult::Failure, context.FailedTransactionStatuses[1]);
	}

	// endregion

	// region transaction - skipped

	TEST(TRANSACTION_TEST_CLASS, CanValidateMultipleTransactionsWithSomeSkipped) {
		// Arrange:
		TransactionTestContext context;
		auto elements = TransactionTraits::CreateMultipleEntityElements();
		elements[1].ResultSeverity = disruptor::ConsumerResultSeverity::Neutral;

		// Act:
		auto result = context.Consumer(elements);

		// Assert:
		test::AssertContinued(result);
		TransactionTraits::AssertEntities(FilterEntityInfos(elements, { 0, 2, 3 }), context.pPolicy->params());
		AssertSkipped(elements, { 1 }, { disruptor::ConsumerResultSeverity::Neutral });
		EXPECT_TRUE(context.FailedTransactionStatuses.empty());
	}

	TEST(TRANSACTION_TEST_CLASS, CanValidateMultipleTransactionsWithSomeSkippedWithPartialFailure) {
		// Arrange:
		TransactionTestContext context;
		auto elements = TransactionTraits::CreateMultipleEntityElements();
		elements[1].ResultSeverity = disruptor::ConsumerResultSeverity::Neutral;
		context.pPolicy->setResult({ ValidationResult::Success, ValidationResult::Failure, ValidationResult::Success });

		// Act:
		auto result = context.Consumer(elements);

		// Assert:
		test::AssertAborted(result, ValidationResult::Failure, disruptor::ConsumerResultSeverity::Fatal);
		TransactionTraits::AssertEntities(FilterEntityInfos(elements, { 0, 2, 3 }), context.pPolicy->params());
		AssertSkipped(elements, { 1, 2 }, { disruptor::ConsumerResultSeverity::Neutral, disruptor::ConsumerResultSeverity::Failure });

		// - notice that the 2nd result failure correctly mapped to the 3rd element
		//   (results are only generated for elements that are not skipped initially)
		ASSERT_EQ(1u, context.FailedTransactionStatuses.size());
		EXPECT_EQ_STATUS(elements[2], ValidationResult::Failure, context.FailedTransactionStatuses[0]);
	}

	// endregion
}}

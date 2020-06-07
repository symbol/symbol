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

#include "partialtransaction/src/chain/PtUpdater.h"
#include "partialtransaction/src/chain/PtValidator.h"
#include "plugins/txes/aggregate/src/model/AggregateTransaction.h"
#include "catapult/cache_tx/MemoryPtCache.h"
#include "catapult/model/TransactionStatus.h"
#include "catapult/thread/FutureUtils.h"
#include "catapult/utils/MemoryUtils.h"
#include "catapult/utils/SpinLock.h"
#include "partialtransaction/tests/test/AggregateTransactionTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/other/ValidationResultTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

#define TEST_CLASS PtUpdaterTests

#define EXPECT_EQ_TRANSACTION_UPDATE_RESULT(RESULT, UPDATE_TYPE, NUM_COSIGNATURES_ADDED) \
	do { \
		EXPECT_EQ(TransactionUpdateResult::UpdateType::UPDATE_TYPE, RESULT.Type); \
		EXPECT_EQ(static_cast<size_t>(NUM_COSIGNATURES_ADDED), RESULT.NumCosignaturesAdded); \
	} while (false)

	namespace {
		model::TransactionInfo CreateRandomTransactionInfo(const std::shared_ptr<const model::Transaction>& pTransaction) {
			auto transactionInfo = test::CreateRandomTransactionInfo();
			transactionInfo.pEntity = pTransaction;
			return transactionInfo;
		}

		std::shared_ptr<model::AggregateTransaction> CreateRandomAggregateTransaction(uint32_t numCosignatures) {
			return test::CreateRandomAggregateTransactionWithCosignatures(numCosignatures);
		}

		// region ExpectedValidatorCalls

		class ExactMatchPredicate {
		public:
			ExactMatchPredicate() : ExactMatchPredicate(0)
			{}

			explicit ExactMatchPredicate(size_t value) : m_value(value)
			{}

		public:
			bool operator()(size_t value) const {
				return m_value == value;
			}

			std::string description(size_t value) const {
				std::ostringstream out;
				out << m_value << " == " << value << " (actual)";
				return out.str();
			}

		private:
			size_t m_value;
		};

		class InclusiveRangeMatchPredicate {
		public:
			InclusiveRangeMatchPredicate() : InclusiveRangeMatchPredicate(0, 0)
			{}

			InclusiveRangeMatchPredicate(size_t minValue, size_t maxValue)
					: m_minValue(minValue)
					, m_maxValue(maxValue)
			{}

		public:
			bool operator()(size_t value) const {
				return m_minValue <= value && value <= m_maxValue;
			}

			std::string description(size_t value) const {
				std::ostringstream out;
				out << m_minValue << "<= " << value << " (actual) <= " << m_maxValue;
				return out.str();
			}

		private:
			size_t m_minValue;
			size_t m_maxValue;
		};

		class AnyMatchPredicate {
		public:
			explicit AnyMatchPredicate(size_t value = 0) : m_isExactMatch(true) {
				setExactMatch(value);
			}

		public:
			void setExactMatch(size_t value) {
				m_exactMatchPredicate = ExactMatchPredicate(value);
			}

			void setInclusiveRangeMatch(size_t minValue, size_t maxValue) {
				m_inclusiveRangeMatchPredicate = InclusiveRangeMatchPredicate(minValue, maxValue);
				m_isExactMatch = false;
			}

		public:
			bool operator()(size_t value) const {
				return m_isExactMatch ? m_exactMatchPredicate(value) : m_inclusiveRangeMatchPredicate(value);
			}

			std::string description(size_t value) const {
				return m_isExactMatch ? m_exactMatchPredicate.description(value) : m_inclusiveRangeMatchPredicate.description(value);
			}

		private:
			bool m_isExactMatch;
			ExactMatchPredicate m_exactMatchPredicate;
			InclusiveRangeMatchPredicate m_inclusiveRangeMatchPredicate;
		};

		struct ExpectedValidatorCalls {
		public:
			ExpectedValidatorCalls() = default;

			ExpectedValidatorCalls(size_t numValidatePartialCalls, size_t numValidateCosignatoriesCalls, size_t numLastCosignatories)
					: NumValidatePartialCalls(numValidatePartialCalls)
					, NumValidateCosignatoriesCalls(numValidateCosignatoriesCalls)
					, NumLastCosignatories(numLastCosignatories)
			{}

		public:
			AnyMatchPredicate NumValidatePartialCalls;
			AnyMatchPredicate NumValidateCosignatoriesCalls;
			AnyMatchPredicate NumLastCosignatories;
		};

		// endregion

		// region ValidateCosignatoriesResultTrigger

		class ValidateCosignatoriesResultTrigger {
		public:
			ValidateCosignatoriesResultTrigger(size_t id) : m_idMatch(id)
			{}

			ValidateCosignatoriesResultTrigger(Key signer)
					: m_idMatch(0) // notice that calls are 1-based, so this will never match
					, m_signer(signer)
			{}

		public:
			bool operator()(const test::CosignaturesMap& cosignaturesMap, size_t id) const {
				return m_idMatch(id) || cosignaturesMap.cend() != cosignaturesMap.find(m_signer);
			}

		private:
			AnyMatchPredicate m_idMatch;
			Key m_signer;
		};

		// endregion

		// region MockPtValidator

		constexpr auto Validate_Partial_Raw_Result = test::MakeValidationResult(validators::ResultSeverity::Failure, 12);
		constexpr auto Validate_Cosignatories_Raw_Result = test::MakeValidationResult(validators::ResultSeverity::Failure, 24);

		class MockPtValidator : public PtValidator {
		public:
			MockPtValidator()
					: m_validatePartialResult(true)
					, m_shouldSleepInValidateCosignatories(false)
					, m_numValidatePartialCalls(0)
					, m_numValidateCosignatoriesCalls(0)
					, m_numLastCosignatories(0)
			{}

		public:
			void setValidatePartialFailure() {
				m_validatePartialResult = false;
			}

			void setValidateCosignatoriesResult(CosignatoriesValidationResult result, const ValidateCosignatoriesResultTrigger& trigger) {
				m_validateCosignatoriesResultTriggers.emplace_back(trigger, result);
			}

			void sleepInValidateCosignatories() {
				m_shouldSleepInValidateCosignatories = true;
			}

		public:
			Result<bool> validatePartial(const model::WeakEntityInfoT<model::Transaction>& transactionInfo) const override {
				utils::SpinLockGuard guard(m_lock);
				++m_numValidatePartialCalls;
				m_transactions.push_back(test::CopyEntity(transactionInfo.entity()));
				m_transactionHashes.push_back(transactionInfo.hash());
				return { Validate_Partial_Raw_Result, m_validatePartialResult };
			}

			Result<CosignatoriesValidationResult> validateCosignatories(
					const model::WeakCosignedTransactionInfo& transactionInfo) const override {
				test::CosignaturesMap cosignaturesMap;
				{
					utils::SpinLockGuard guard(m_lock);
					++m_numValidateCosignatoriesCalls;
					m_transactions.push_back(test::CopyEntity(transactionInfo.transaction()));

					// there shouldn't be any duplicate cosignatories
					m_numLastCosignatories = transactionInfo.cosignatures().size();
					cosignaturesMap = test::ToMap(transactionInfo.cosignatures());
				}

				if (m_shouldSleepInValidateCosignatories) {
					// sleep for a random amount of time to exploit potential race conditions
					auto sleepMs = test::RandomByte() / 2;
					CATAPULT_LOG(debug) << "sleeping for " << static_cast<int>(sleepMs) << "ms";
					test::Sleep(sleepMs);
				}

				return { Validate_Cosignatories_Raw_Result, getCosignatoriesValidationResult(cosignaturesMap) };
			}

		private:
			CosignatoriesValidationResult getCosignatoriesValidationResult(const test::CosignaturesMap& cosignaturesMap) const {
				for (const auto& pair : m_validateCosignatoriesResultTriggers) {
					if (pair.first(cosignaturesMap, m_numValidateCosignatoriesCalls))
						return pair.second;
				}

				return CosignatoriesValidationResult::Missing;
			}

		public:
			void reset() {
				m_numValidatePartialCalls = 0;
				m_numValidateCosignatoriesCalls = 0;
				m_numLastCosignatories = 0;
				m_transactions.clear();
				m_transactionHashes.clear();
			}

			void assertCalls(const model::AggregateTransaction& aggregateTransaction, const ExpectedValidatorCalls& expected) {
				// Assert: check calls
				assertCallsImpl(aggregateTransaction, expected);

				// - other overload should be used if there are verify partial calls
				EXPECT_TRUE(expected.NumValidatePartialCalls(0));
			}

			void assertCalls(
					const model::AggregateTransaction& aggregateTransaction,
					const Hash256& aggregateTransactionHash,
					const ExpectedValidatorCalls& expected) {
				// Assert: check calls
				assertCallsImpl(aggregateTransaction, expected);

				// - check forwarded hashes
				for (const auto& entityHash : m_transactionHashes)
					EXPECT_EQ(aggregateTransactionHash, entityHash);

				// - other overload should be used if there are no verify partial calls
				EXPECT_FALSE(expected.NumValidatePartialCalls(0));
			}

		private:
			void assertCallsImpl(const model::AggregateTransaction& aggregateTransaction, const ExpectedValidatorCalls& expected) {
				// Assert: check calls
				CATAPULT_LOG(debug)
						<< "NumValidatePartialCalls = " << m_numValidatePartialCalls
						<< ", NumValidateCosignatoriesCalls = " << m_numValidateCosignatoriesCalls
						<< ", NumLastCosignatories = " << m_numLastCosignatories;
				CheckPredicate(expected.NumValidatePartialCalls, m_numValidatePartialCalls, "NumValidatePartialCalls");
				CheckPredicate(expected.NumValidateCosignatoriesCalls, m_numValidateCosignatoriesCalls, "NumValidateCosignatoriesCalls");
				CheckPredicate(expected.NumLastCosignatories, m_numLastCosignatories, "NumLastCosignatories");

				// - check forwarded transactions
				auto pTransactionWithoutCosignatures = test::StripCosignatures(aggregateTransaction);
				for (const auto& pTransaction : m_transactions)
					EXPECT_EQ(*pTransactionWithoutCosignatures, *pTransaction);
			}

			static void CheckPredicate(const AnyMatchPredicate& predicate, size_t value, const char* message) {
				EXPECT_TRUE(predicate(value)) << message << ": " << predicate.description(value);
			}

		private:
			using ValidateCosignatoriesResultTriggers = std::vector<std::pair<
				ValidateCosignatoriesResultTrigger,
				CosignatoriesValidationResult>>;

		private:
			bool m_validatePartialResult;
			bool m_shouldSleepInValidateCosignatories;
			ValidateCosignatoriesResultTriggers m_validateCosignatoriesResultTriggers;

			mutable utils::SpinLock m_lock;
			mutable size_t m_numValidatePartialCalls;
			mutable std::atomic<size_t> m_numValidateCosignatoriesCalls; // accessed outside of lock by getCosignatoriesValidationResult
			mutable size_t m_numLastCosignatories;
			mutable std::vector<std::unique_ptr<model::Transaction>> m_transactions;
			mutable std::vector<Hash256> m_transactionHashes;
		};

		// endregion

		// region test context

		class UpdaterTestContext {
		public:
			UpdaterTestContext()
					: m_transactionsCache(cache::MemoryCacheOptions(1024, 1000))
					, m_pUniqueValidator(std::make_unique<MockPtValidator>())
					, m_pValidator(m_pUniqueValidator.get())
					, m_pPool(test::CreateStartedIoThreadPool())
					, m_pUpdater(std::make_unique<PtUpdater>(
							m_transactionsCache,
							std::move(m_pUniqueValidator),
							PtUpdater::CompletedTransactionSink([this](auto&& pTransaction) {
								m_completedTransactions.push_back(std::move(pTransaction));
							}),
							[this](const auto& transaction, const auto& hash, auto result) {
								// notice that transaction.Deadline is used as transaction marker
								m_failedTransactionStatuses.emplace_back(hash, transaction.Deadline, utils::to_underlying_type(result));
							},
							*m_pPool))
			{}

			~UpdaterTestContext() {
				m_pPool->join();
			}

		public:
			const auto& transactionsCache() const {
				return m_transactionsCache;
			}

			const auto& completedTransactions() const {
				return m_completedTransactions;
			}

			const auto& failedTransactionStatuses() const {
				return m_failedTransactionStatuses;
			}

			auto& validator() {
				return *m_pValidator;
			}

			auto& updater() {
				return *m_pUpdater;
			}

		public:
			void destroyUpdater() {
				m_pUpdater.reset();
			}

			auto numWorkerThreads() {
				return m_pPool->numWorkerThreads();
			}

		public:
			// asserts that the pt cache contains a \em single \a aggregateTransaction with \a aggregateHash and \a cosignatures
			void assertSingleTransactionInCache(
					const Hash256& aggregateHash,
					const model::AggregateTransaction& aggregateTransaction,
					const std::vector<model::Cosignature>& cosignatures) const {
				// Assert:
				EXPECT_EQ(1u, m_transactionsCache.view().size());

				assertTransactionInCache(aggregateHash, aggregateTransaction, cosignatures);
			}

		private:
			void assertTransactionInCache(
					const Hash256& aggregateHash,
					const model::AggregateTransaction& aggregateTransaction,
					const std::vector<model::Cosignature>& cosignatures) const {
				// Assert: expected transaction is in the cache
				auto view = m_transactionsCache.view();
				auto transactionInfoFromCache = view.find(aggregateHash);
				ASSERT_TRUE(!!transactionInfoFromCache);

				// - all cosignatures are present
				EXPECT_EQ(cosignatures.size(), transactionInfoFromCache.cosignatures().size());

				auto expectedCosignaturesMap = test::ToMap(cosignatures);
				for (const auto& cosignature : transactionInfoFromCache.cosignatures()) {
					auto message = "cosignatory " + test::ToString(cosignature.SignerPublicKey);

					auto iter = expectedCosignaturesMap.find(cosignature.SignerPublicKey);
					ASSERT_NE(expectedCosignaturesMap.cend(), iter) << message;
					EXPECT_EQ(iter->first, cosignature.SignerPublicKey) << message;
					EXPECT_EQ(iter->second, cosignature.Signature) << message;
					expectedCosignaturesMap.erase(iter);
				}

				EXPECT_TRUE(expectedCosignaturesMap.empty());

				// - transaction is stripped of all cosignatures
				auto pTransactionWithoutCosignatures = test::StripCosignatures(aggregateTransaction);
				EXPECT_EQ(*pTransactionWithoutCosignatures, transactionInfoFromCache.transaction());
			}

		public:
			// asserts that the transaction cache contains correct extended properties for transaction info (\a transactionInfo)
			// \note this will modify the cache
			void assertTransactionInCacheHasCorrectExtendedProperties(const model::TransactionInfo& transactionInfo) {
				// Assert: expected transaction is in the cache (remove is only way to get *full* cache info)
				auto modifier = m_transactionsCache.modifier();
				auto transactionInfoFromCache = modifier.remove(transactionInfo.EntityHash);
				ASSERT_TRUE(!!transactionInfoFromCache);

				// - the cache should preserve extracted addresses - neither the updater nor the tests modify the extracted addresses,
				//   so the info in the cache should point to the same addresses as the first matching info that was added (the parameter)
				EXPECT_TRUE(!!transactionInfoFromCache.OptionalExtractedAddresses);
				EXPECT_EQ(transactionInfo.OptionalExtractedAddresses.get(), transactionInfoFromCache.OptionalExtractedAddresses.get());
			}

		public:
			// asserts that the failed transaction callback was called with \a expectedResult for transaction info (\a transactionInfo)
			void assertSingleFailedTransaction(
					const model::TransactionInfo& transactionInfo,
					validators::ValidationResult expectedResult) {
				// Assert:
				ASSERT_EQ(1u, m_failedTransactionStatuses.size());

				const auto& status = m_failedTransactionStatuses[0];
				EXPECT_EQ(utils::to_underlying_type(expectedResult), status.Status);

				EXPECT_EQ(transactionInfo.pEntity->Deadline, status.Deadline);
				EXPECT_EQ(transactionInfo.EntityHash, status.Hash);
			}

		private:
			cache::MemoryPtCacheProxy m_transactionsCache;

			std::unique_ptr<MockPtValidator> m_pUniqueValidator; // moved into m_pUpdater
			MockPtValidator* m_pValidator;
			std::vector<std::unique_ptr<model::Transaction>> m_completedTransactions;

			std::unique_ptr<thread::IoThreadPool> m_pPool;
			std::unique_ptr<PtUpdater> m_pUpdater; // unique_ptr for destroyUpdater

			std::vector<model::TransactionStatus> m_failedTransactionStatuses;
		};

		// endregion

		template<typename TAction>
		void RunTestWithTransactionInCache(uint32_t numCosignatures, TAction action) {
			// Arrange:
			UpdaterTestContext context;

			// - add a transaction
			auto pTransaction = CreateRandomAggregateTransaction(numCosignatures);
			auto transactionInfo = CreateRandomTransactionInfo(pTransaction);
			test::FixCosignatures(transactionInfo.EntityHash, *pTransaction);
			context.updater().update(transactionInfo).get();
			context.validator().reset();

			// Act:
			action(context, transactionInfo, *pTransaction);
		}
	}

	// region update transaction - invalid aggregate

	TEST(TEST_CLASS, CannotAddNonAggregateTransaction) {
		// Arrange:
		UpdaterTestContext context;
		auto transactionInfo = test::CreateRandomTransactionInfo();

		// Act + Assert:
		EXPECT_THROW(context.updater().update(transactionInfo), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CannotAddInvalidAggregateTransaction) {
		// Arrange:
		UpdaterTestContext context;
		auto pTransaction = CreateRandomAggregateTransaction(2);
		auto transactionInfo = CreateRandomTransactionInfo(pTransaction);

		// - mark the transaction as invalid
		context.validator().setValidatePartialFailure();

		// Act:
		auto result = context.updater().update(transactionInfo).get();

		// Assert:
		EXPECT_EQ_TRANSACTION_UPDATE_RESULT(result, Invalid, 0);
		EXPECT_EQ(0u, context.transactionsCache().view().size());

		EXPECT_TRUE(context.completedTransactions().empty());
		context.assertSingleFailedTransaction(transactionInfo, Validate_Partial_Raw_Result);
		context.validator().assertCalls(*pTransaction, transactionInfo.EntityHash, { 1, 0, 0 });
	}

	// endregion

	// region update transaction - add (new) complete + incomplete

	TEST(TEST_CLASS, CanAddIncompleteAggregateWithoutCosignatures) {
		// Arrange:
		UpdaterTestContext context;
		auto pTransaction = CreateRandomAggregateTransaction(0);
		auto transactionInfo = CreateRandomTransactionInfo(pTransaction);

		// Act:
		auto result = context.updater().update(transactionInfo).get();

		// Assert:
		EXPECT_EQ_TRANSACTION_UPDATE_RESULT(result, New, 0);
		context.assertSingleTransactionInCache(transactionInfo.EntityHash, *pTransaction, {});
		context.assertTransactionInCacheHasCorrectExtendedProperties(transactionInfo);

		EXPECT_TRUE(context.completedTransactions().empty());
		EXPECT_TRUE(context.failedTransactionStatuses().empty());
		context.validator().assertCalls(*pTransaction, transactionInfo.EntityHash, { 1, 1, 0 });
	}

	TEST(TEST_CLASS, CanAddIncompleteAggregateWithCosignatures) {
		// Arrange:
		UpdaterTestContext context;
		auto pTransaction = CreateRandomAggregateTransaction(3);
		auto transactionInfo = CreateRandomTransactionInfo(pTransaction);
		test::FixCosignatures(transactionInfo.EntityHash, *pTransaction);

		// Act:
		auto result = context.updater().update(transactionInfo).get();

		// Assert:
		EXPECT_EQ_TRANSACTION_UPDATE_RESULT(result, New, 3);

		const auto* pCosignatures = pTransaction->CosignaturesPtr();
		context.assertSingleTransactionInCache(
				transactionInfo.EntityHash,
				*pTransaction,
				{ pCosignatures[0], pCosignatures[1], pCosignatures[2] });
		context.assertTransactionInCacheHasCorrectExtendedProperties(transactionInfo);

		EXPECT_TRUE(context.completedTransactions().empty());
		EXPECT_TRUE(context.failedTransactionStatuses().empty());
		context.validator().assertCalls(*pTransaction, transactionInfo.EntityHash, { 1, 6, 3 });
	}

	TEST(TEST_CLASS, CanAddCompleteAggregateWithoutCosignatures) {
		// Arrange: this test simulates an aggregate that requires a single signature (and no cosignatures)
		UpdaterTestContext context;
		auto pTransaction = CreateRandomAggregateTransaction(0);
		auto transactionInfo = CreateRandomTransactionInfo(pTransaction);

		// - mark the transaction as complete
		context.validator().setValidateCosignatoriesResult(CosignatoriesValidationResult::Success, 1);

		// Act:
		auto result = context.updater().update(transactionInfo).get();

		// Assert:
		EXPECT_EQ_TRANSACTION_UPDATE_RESULT(result, New, 0);

		EXPECT_EQ(0u, context.transactionsCache().view().size());

		ASSERT_EQ(1u, context.completedTransactions().size());
		test::AssertStitchedTransaction(*context.completedTransactions()[0], *pTransaction, {});
		EXPECT_TRUE(context.failedTransactionStatuses().empty());
		context.validator().assertCalls(*pTransaction, transactionInfo.EntityHash, { 1, 1, 0 });
	}

	TEST(TEST_CLASS, CanAddCompleteAggregateWithCosignatures) {
		// Arrange:
		UpdaterTestContext context;
		auto pTransaction = CreateRandomAggregateTransaction(3);
		auto transactionInfo = CreateRandomTransactionInfo(pTransaction);
		test::FixCosignatures(transactionInfo.EntityHash, *pTransaction);

		// - mark the transaction as complete
		context.validator().setValidateCosignatoriesResult(CosignatoriesValidationResult::Success, 6);

		// Act:
		auto result = context.updater().update(transactionInfo).get();

		// Assert:
		EXPECT_EQ_TRANSACTION_UPDATE_RESULT(result, New, 3);

		EXPECT_EQ(0u, context.transactionsCache().view().size());

		const auto* pCosignatures = pTransaction->CosignaturesPtr();
		ASSERT_EQ(1u, context.completedTransactions().size());
		test::AssertStitchedTransaction(*context.completedTransactions()[0], *pTransaction, {
			pCosignatures[0], pCosignatures[1], pCosignatures[2]
		});
		EXPECT_TRUE(context.failedTransactionStatuses().empty());
		context.validator().assertCalls(*pTransaction, transactionInfo.EntityHash, { 1, 6, 3 });
	}

	// endregion

	// region update transaction - add (existing) complete + incomplete

	namespace {
		model::TransactionInfo CopyAndReplaceTransaction(
				const model::TransactionInfo& transactionInfo,
				const std::shared_ptr<model::Transaction>& pTransaction) {
			auto copy = transactionInfo.copy();
			copy.pEntity = pTransaction;
			return copy;
		}
	}

	TEST(TEST_CLASS, AddingExistingAggregateWithCosignaturesMergesCosignatures) {
		// Arrange:
		RunTestWithTransactionInCache(3, [](auto& context, const auto& transactionInfo1, const auto& transaction1) {
			// Act: add a second transaction with same hash
			auto pTransaction2 = CreateRandomAggregateTransaction(2);
			auto transactionInfo2 = CopyAndReplaceTransaction(transactionInfo1, pTransaction2);
			test::FixCosignatures(transactionInfo2.EntityHash, *pTransaction2);
			auto result = context.updater().update(transactionInfo2).get();

			// Assert: the original transaction (for the test the tx data is different) should be used with merged cosignatures
			EXPECT_EQ_TRANSACTION_UPDATE_RESULT(result, Existing, 2);

			const auto* pCosignatures1 = transaction1.CosignaturesPtr();
			const auto* pCosignatures2 = pTransaction2->CosignaturesPtr();
			context.assertSingleTransactionInCache(transactionInfo1.EntityHash, transaction1, {
				pCosignatures1[0], pCosignatures1[1], pCosignatures1[2],
				pCosignatures2[0], pCosignatures2[1]
			});
			context.assertTransactionInCacheHasCorrectExtendedProperties(transactionInfo1);

			EXPECT_TRUE(context.completedTransactions().empty());
			EXPECT_TRUE(context.failedTransactionStatuses().empty());
			context.validator().assertCalls(transaction1, { 0, 4, 3 + 2 });
		});
	}

	TEST(TEST_CLASS, AddingExistingAggregateWithCosignaturesMergesCosignaturesAndIgnoresDuplicates) {
		// Arrange:
		RunTestWithTransactionInCache(3, [](auto& context, const auto& transactionInfo1, const auto& transaction1) {
			// Act: add a second transaction with same hash and a duplicate cosignature
			auto pTransaction2 = CreateRandomAggregateTransaction(3);
			auto transactionInfo2 = CopyAndReplaceTransaction(transactionInfo1, pTransaction2);
			test::FixCosignatures(transactionInfo2.EntityHash, *pTransaction2);
			pTransaction2->CosignaturesPtr()[1] = transaction1.CosignaturesPtr()[2];
			auto result = context.updater().update(transactionInfo2).get();

			// Assert: the original transaction (for the test the tx data is different) should be used with merged cosignatures
			EXPECT_EQ_TRANSACTION_UPDATE_RESULT(result, Existing, 2);

			const auto* pCosignatures1 = transaction1.CosignaturesPtr();
			const auto* pCosignatures2 = pTransaction2->CosignaturesPtr();
			context.assertSingleTransactionInCache(transactionInfo1.EntityHash, transaction1, {
				pCosignatures1[0], pCosignatures1[1], pCosignatures1[2],
				pCosignatures2[0], pCosignatures2[2]
			});
			context.assertTransactionInCacheHasCorrectExtendedProperties(transactionInfo1);

			EXPECT_TRUE(context.completedTransactions().empty());
			EXPECT_TRUE(context.failedTransactionStatuses().empty());
			context.validator().assertCalls(transaction1, { 0, 4, 3 + 2 });
		});
	}

	TEST(TEST_CLASS, AddingExistingAggregateWithCosignaturesCanCompleteTransaction) {
		// Arrange:
		RunTestWithTransactionInCache(3, [](auto& context, const auto& transactionInfo1, const auto& transaction1) {
			// - mark the transaction as complete
			context.validator().setValidateCosignatoriesResult(CosignatoriesValidationResult::Success, 4);

			// Act: add a second transaction with same hash
			auto pTransaction2 = CreateRandomAggregateTransaction(2);
			auto transactionInfo2 = CopyAndReplaceTransaction(transactionInfo1, pTransaction2);
			test::FixCosignatures(transactionInfo2.EntityHash, *pTransaction2);
			auto result = context.updater().update(transactionInfo2).get();

			// Assert: the original transaction (for the test the tx data is different) should be used with merged cosignatures
			EXPECT_EQ_TRANSACTION_UPDATE_RESULT(result, Existing, 2);

			const auto* pCosignatures1 = transaction1.CosignaturesPtr();
			const auto* pCosignatures2 = pTransaction2->CosignaturesPtr();
			EXPECT_EQ(0u, context.transactionsCache().view().size());

			ASSERT_EQ(1u, context.completedTransactions().size());
			test::AssertStitchedTransaction(*context.completedTransactions()[0], transaction1, {
				pCosignatures1[0], pCosignatures1[1], pCosignatures1[2],
				pCosignatures2[0], pCosignatures2[1]
			});
			EXPECT_TRUE(context.failedTransactionStatuses().empty());
			context.validator().assertCalls(transaction1, { 0, 4, 3 + 2 });
		});
	}

	// endregion

	// region update transaction - add invalid cosignatures

	namespace {
		template<typename TCorruptCosignature>
		void RunTransactionWithInvalidCosignatureTest(
				size_t numIneligibleCosignatories,
				bool isRejectedInCheckEligibility,
				TCorruptCosignature corruptCosignature) {
			// Arrange:
			UpdaterTestContext context;
			auto pTransaction = CreateRandomAggregateTransaction(3);
			auto transactionInfo = CreateRandomTransactionInfo(pTransaction);
			test::FixCosignatures(transactionInfo.EntityHash, *pTransaction);

			// - mark a cosignatory as invalid
			corruptCosignature(context, pTransaction->CosignaturesPtr()[1]);

			// Act:
			auto result = context.updater().update(transactionInfo).get();

			// Assert: the invalid cosignature should not have been added
			EXPECT_EQ_TRANSACTION_UPDATE_RESULT(result, New, 2);

			const auto* pCosignatures = pTransaction->CosignaturesPtr();
			context.assertSingleTransactionInCache(transactionInfo.EntityHash, *pTransaction, { pCosignatures[0], pCosignatures[2] });
			context.assertTransactionInCacheHasCorrectExtendedProperties(transactionInfo);

			EXPECT_TRUE(context.completedTransactions().empty());
			EXPECT_TRUE(context.failedTransactionStatuses().empty());

			ExpectedValidatorCalls expectedValidatorCalls;
			expectedValidatorCalls.NumValidatePartialCalls.setExactMatch(1); // 1 (transaction isValid)
			// * 1 x 3 (cosig checkEligibility) + 1 x numIneligibleCosignatories (ineligible-cosig checkEligibility)
			// * 1 x 2 (valid-cosig isComplete)
			expectedValidatorCalls.NumValidateCosignatoriesCalls.setExactMatch(5 + numIneligibleCosignatories);

			// * 1: { Valid, Valid, Invalid } - last call only with ineligible cosignature
			// * 2: { Valid, Invalid, Valid }, { Invalid, Valid, Valid } - invalid cosignature is excluded from subsequent calls
			// - above is for !!isRejectedInCheckEligibility
			//   when !isRejectedInCheckEligibility, cosignatures are rejected after validateCosignatories call,
			//   which captures NumLastCosignatories, so add one
			auto numLastCosignatoriesDelta = isRejectedInCheckEligibility ? 0u : 1u;
			expectedValidatorCalls.NumLastCosignatories.setInclusiveRangeMatch(
					1 + numLastCosignatoriesDelta,
					2 + numLastCosignatoriesDelta);
			context.validator().assertCalls(*pTransaction, transactionInfo.EntityHash, expectedValidatorCalls);
		}
	}

	TEST(TEST_CLASS, AddingAggregateWithCosignaturesIgnoresIneligibleCosignatures) {
		// Arrange:
		RunTransactionWithInvalidCosignatureTest(1, true, [](auto& context, const auto& cosignature) {
			// - mark a cosignatory as ineligible
			context.validator().setValidateCosignatoriesResult(CosignatoriesValidationResult::Ineligible, cosignature.SignerPublicKey);
		});
	}

	TEST(TEST_CLASS, AddingAggregateWithCosignaturesIgnoresUnverifiableCosignatures) {
		// Arrange:
		// - validateCosignatories is called before signature check, so corrupt cosignature will always be passed to validateCosignatories
		//   where it is captured
		RunTransactionWithInvalidCosignatureTest(0, false, [](const auto&, auto& cosignature) {
			// - corrupt a signature
			cosignature.Signature[0] ^= 0xFF;
		});
	}

	TEST(TEST_CLASS, AddingAggregateWithCosignaturesIgnoresDuplicateCosignatures) {
		// Arrange:
		UpdaterTestContext context;
		auto pTransaction = CreateRandomAggregateTransaction(3);
		auto transactionInfo = CreateRandomTransactionInfo(pTransaction);
		test::FixCosignatures(transactionInfo.EntityHash, *pTransaction);

		// - create a redundant cosignature
		pTransaction->CosignaturesPtr()[0] = pTransaction->CosignaturesPtr()[2];

		// Act:
		auto result = context.updater().update(transactionInfo).get();

		// Assert: the redundant cosignature should not have been added
		EXPECT_EQ_TRANSACTION_UPDATE_RESULT(result, New, 2);

		const auto* pCosignatures = pTransaction->CosignaturesPtr();
		context.assertSingleTransactionInCache(transactionInfo.EntityHash, *pTransaction, { pCosignatures[0], pCosignatures[1] });
		context.assertTransactionInCacheHasCorrectExtendedProperties(transactionInfo);

		EXPECT_TRUE(context.completedTransactions().empty());
		EXPECT_TRUE(context.failedTransactionStatuses().empty());
		context.validator().assertCalls(*pTransaction, transactionInfo.EntityHash, { 1, 4, 2 });
	}

	// endregion

	// region update cosignature - ignored (no matching transaction)

	TEST(TEST_CLASS, AddingCosignatureWithoutMatchingTransactionIsIgnored) {
		// Arrange:
		UpdaterTestContext context;
		auto pTransaction = CreateRandomAggregateTransaction(3);
		auto transactionInfo = CreateRandomTransactionInfo(pTransaction);
		auto cosignature = test::GenerateValidCosignature(transactionInfo.EntityHash);

		// Act:
		auto result = context.updater().update(cosignature).get();

		// Assert: nothing was added to the cache
		EXPECT_EQ(CosignatureUpdateResult::Ineligible, result);
		EXPECT_EQ(0u, context.transactionsCache().view().size());

		EXPECT_TRUE(context.completedTransactions().empty());
		EXPECT_TRUE(context.failedTransactionStatuses().empty());
		context.validator().assertCalls(*pTransaction, { 0, 0, 0 });
	}

	// endregion

	// region update cosignature - add

	TEST(TEST_CLASS, AddingCosignatureWithMatchingTransactionAddsCosignatureToTransaction) {
		// Arrange:
		RunTestWithTransactionInCache(3, [](auto& context, const auto& transactionInfo, const auto& transaction) {
			// - create a compatible cosignature
			auto cosignature = test::GenerateValidCosignature(transactionInfo.EntityHash);

			// Act:
			auto result = context.updater().update(cosignature).get();

			// Assert: the cosignature was added
			EXPECT_EQ(CosignatureUpdateResult::Added_Incomplete, result);

			const auto* pCosignatures = transaction.CosignaturesPtr();
			context.assertSingleTransactionInCache(transactionInfo.EntityHash, transaction, {
				pCosignatures[0], pCosignatures[1], pCosignatures[2],
				cosignature
			});
			context.assertTransactionInCacheHasCorrectExtendedProperties(transactionInfo);

			EXPECT_TRUE(context.completedTransactions().empty());
			EXPECT_TRUE(context.failedTransactionStatuses().empty());
			context.validator().assertCalls(transaction, { 0, 2, 3 + 1 });
		});
	}

	TEST(TEST_CLASS, AddingCosignatureWithMatchingTransactionCanCompleteTransaction) {
		// Arrange:
		RunTestWithTransactionInCache(3, [](auto& context, const auto& transactionInfo, const auto& transaction) {
			// - create a compatible cosignature
			auto cosignature = test::GenerateValidCosignature(transactionInfo.EntityHash);

			// - mark the transaction as complete
			context.validator().setValidateCosignatoriesResult(CosignatoriesValidationResult::Success, 2);

			// Act:
			auto result = context.updater().update(cosignature).get();

			// Assert: the cosignature was added
			EXPECT_EQ(CosignatureUpdateResult::Added_Complete, result);

			EXPECT_EQ(0u, context.transactionsCache().view().size());

			const auto* pCosignatures = transaction.CosignaturesPtr();
			ASSERT_EQ(1u, context.completedTransactions().size());
			test::AssertStitchedTransaction(*context.completedTransactions()[0], transaction, {
				pCosignatures[0], pCosignatures[1], pCosignatures[2],
				cosignature
			});
			EXPECT_TRUE(context.failedTransactionStatuses().empty());
			context.validator().assertCalls(transaction, { 0, 2, 3 + 1 });
		});
	}

	// endregion

	// region update cosignature - invalid

	TEST(TEST_CLASS, AddingCosignatureThatTriggersUnexpectedTransactionFailurePurgesTransactionFromCache) {
		// Arrange:
		RunTestWithTransactionInCache(3, [](auto& context, const auto& transactionInfo, const auto& transaction) {
			// - create a compatible cosignature
			auto cosignature = test::GenerateValidCosignature(transactionInfo.EntityHash);

			// - mark the transaction as failed
			context.validator().setValidateCosignatoriesResult(CosignatoriesValidationResult::Failure, 1);

			// Act:
			auto result = context.updater().update(cosignature).get();

			// Assert: the cosignature triggered a failure
			EXPECT_EQ(CosignatureUpdateResult::Error, result);

			// - the transaction was purged from the cache
			EXPECT_EQ(0u, context.transactionsCache().view().size());

			EXPECT_TRUE(context.completedTransactions().empty());
			context.assertSingleFailedTransaction(transactionInfo, Validate_Cosignatories_Raw_Result);
			context.validator().assertCalls(transaction, { 0, 1, 3 + 1 });
		});
	}

	namespace {
		template<typename TCorruptCosignature>
		void RunAddingInvalidCosignatureTest(
				CosignatureUpdateResult expectedResult,
				const ExpectedValidatorCalls& expectedValidatorCalls,
				TCorruptCosignature corruptCosignature) {
			// Arrange:
			RunTestWithTransactionInCache(3, [=](auto& context, const auto& transactionInfo, const auto& transaction) {
				// - create a compatible cosignature
				auto cosignature = test::GenerateValidCosignature(transactionInfo.EntityHash);

				// - mark the cosignature as invalid
				corruptCosignature(context, cosignature);

				// Act:
				auto result = context.updater().update(cosignature).get();

				// Assert: the cosignature was rejected
				EXPECT_EQ(expectedResult, result);

				const auto* pCosignatures = transaction.CosignaturesPtr();
				context.assertSingleTransactionInCache(transactionInfo.EntityHash, transaction, {
					pCosignatures[0], pCosignatures[1], pCosignatures[2]
				});
				context.assertTransactionInCacheHasCorrectExtendedProperties(transactionInfo);

				EXPECT_TRUE(context.completedTransactions().empty());
				EXPECT_TRUE(context.failedTransactionStatuses().empty());
				context.validator().assertCalls(transaction, expectedValidatorCalls);
			});
		}
	}

	TEST(TEST_CLASS, AddingIneligibleCosignatureWithMatchingTransactionIsIgnored) {
		// Arrange:
		ExpectedValidatorCalls expectedValidatorCalls;
		expectedValidatorCalls.NumValidateCosignatoriesCalls.setExactMatch(2); // 1 (new cosig) + 1 (ineligible cosig)
		expectedValidatorCalls.NumLastCosignatories.setExactMatch(1); // ineligible cosig only
		RunAddingInvalidCosignatureTest(CosignatureUpdateResult::Ineligible, expectedValidatorCalls, [](
				auto& context,
				const auto& cosignature) {
			// - mark the cosignature as ineligible
			context.validator().setValidateCosignatoriesResult(CosignatoriesValidationResult::Ineligible, cosignature.SignerPublicKey);
		});
	}

	TEST(TEST_CLASS, AddingUnverifiableCosignatureWithMatchingTransactionIsIgnored) {
		// Arrange:
		ExpectedValidatorCalls expectedValidatorCalls;
		expectedValidatorCalls.NumValidateCosignatoriesCalls.setExactMatch(1); // 1 (new cosig)
		expectedValidatorCalls.NumLastCosignatories.setExactMatch(4); // 3 (existing cosigs) + 1 (new cosig)
		RunAddingInvalidCosignatureTest(CosignatureUpdateResult::Unverifiable, expectedValidatorCalls, [](const auto&, auto& cosignature) {
			// - make the cosignature unverifiable
			cosignature.Signature[0] ^= 0xFF;
		});
	}

	TEST(TEST_CLASS, AddingDuplicateCosignatureWithMatchingTransactionIsIgnored) {
		// Arrange:
		RunTestWithTransactionInCache(3, [](auto& context, const auto& transactionInfo, const auto& transaction) {
			// Act: add an existing cosignature
			const auto& existingCosignature = transaction.CosignaturesPtr()[1];
			auto result = context.updater().update({
				existingCosignature.SignerPublicKey,
				existingCosignature.Signature,
				transactionInfo.EntityHash
			}).get();

			// Assert: the duplicate cosignature was ignored
			EXPECT_EQ(CosignatureUpdateResult::Redundant, result);

			const auto* pCosignatures = transaction.CosignaturesPtr();
			context.assertSingleTransactionInCache(transactionInfo.EntityHash, transaction, {
				pCosignatures[0], pCosignatures[1], pCosignatures[2]
			});
			context.assertTransactionInCacheHasCorrectExtendedProperties(transactionInfo);

			EXPECT_TRUE(context.completedTransactions().empty());
			EXPECT_TRUE(context.failedTransactionStatuses().empty());
			context.validator().assertCalls(transaction, { 0, 0, 0 });
		});
	}

	// endregion

	// region update cosignature - stale cosignature detected

	TEST(TEST_CLASS, StaleCosignatureIsPurgedWhenNewValidCosignatureIsAdded) {
		// Arrange:
		RunTestWithTransactionInCache(3, [](auto& context, const auto& transactionInfo, const auto& transaction) {
			// - create a compatible cosignature
			auto cosignature = test::GenerateValidCosignature(transactionInfo.EntityHash);

			// - change an already accepted and valid cosignature to be ineligible
			//   this simulates edge case where: (1) account C1 cosigns, (2) account C1 is converted to multisig and thus invalid
			context.validator().setValidateCosignatoriesResult(
					CosignatoriesValidationResult::Ineligible,
					transaction.CosignaturesPtr()[1].SignerPublicKey);

			// Act:
			auto result = context.updater().update(cosignature).get();

			// Assert: the cosignature was added
			EXPECT_EQ(CosignatureUpdateResult::Added_Incomplete, result);

			const auto* pCosignatures = transaction.CosignaturesPtr();
			context.assertSingleTransactionInCache(transactionInfo.EntityHash, transaction, {
				pCosignatures[0], pCosignatures[2],
				cosignature
			});
			context.assertTransactionInCacheHasCorrectExtendedProperties(transactionInfo);

			EXPECT_TRUE(context.completedTransactions().empty());
			EXPECT_TRUE(context.failedTransactionStatuses().empty());

			ExpectedValidatorCalls expectedValidatorCalls;
			// * 1 (new cosig) + 1 (ineligible) + 3 (existing cosig) + 1 (isComplete)
			expectedValidatorCalls.NumValidateCosignatoriesCalls.setExactMatch(6);
			expectedValidatorCalls.NumLastCosignatories.setExactMatch(3); // 1 (new cosig) + 2 (existing valid cosig)
			context.validator().assertCalls(transaction, expectedValidatorCalls);
		});
	}

	TEST(TEST_CLASS, MultipleStaleCosignaturesArePurgedWhenNewValidCosignatureIsAdded) {
		// Arrange:
		RunTestWithTransactionInCache(3, [](auto& context, const auto& transactionInfo, const auto& transaction) {
			// - create a compatible cosignature
			auto cosignature = test::GenerateValidCosignature(transactionInfo.EntityHash);

			// - change two already accepted and valid cosignatures to be ineligible
			//   this simulates edge case where: (1) account C1 cosigns, (2) account C1 is converted to multisig and thus invalid
			context.validator().setValidateCosignatoriesResult(
					CosignatoriesValidationResult::Ineligible,
					transaction.CosignaturesPtr()[0].SignerPublicKey);
			context.validator().setValidateCosignatoriesResult(
					CosignatoriesValidationResult::Ineligible,
					transaction.CosignaturesPtr()[2].SignerPublicKey);

			// Act:
			auto result = context.updater().update(cosignature).get();

			// Assert: the cosignature was added
			EXPECT_EQ(CosignatureUpdateResult::Added_Incomplete, result);

			const auto* pCosignatures = transaction.CosignaturesPtr();
			context.assertSingleTransactionInCache(transactionInfo.EntityHash, transaction, {
				pCosignatures[1],
				cosignature
			});
			context.assertTransactionInCacheHasCorrectExtendedProperties(transactionInfo);

			EXPECT_TRUE(context.completedTransactions().empty());
			EXPECT_TRUE(context.failedTransactionStatuses().empty());

			ExpectedValidatorCalls expectedValidatorCalls;
			// * 1 (new cosig) + 1 (ineligible) + 3 (existing cosig) + 1 (isComplete)
			expectedValidatorCalls.NumValidateCosignatoriesCalls.setExactMatch(6);
			expectedValidatorCalls.NumLastCosignatories.setExactMatch(2); // 1 (new cosig) + 1 (existing valid cosig)
			context.validator().assertCalls(transaction, expectedValidatorCalls);
		});
	}

	TEST(TEST_CLASS, StaleCosignatureIsPurgedWhenNewUnverifiableCosignatureIsAdded) {
		// Arrange:
		RunTestWithTransactionInCache(3, [](auto& context, const auto& transactionInfo, const auto& transaction) {
			// - create an unverifiable cosignature
			auto cosignature = test::GenerateValidCosignature(transactionInfo.EntityHash);
			cosignature.Signature[0] ^= 0xFF;

			// - change an already accepted and valid cosignature to be ineligible
			//   this simulates edge case where: (1) account C1 cosigns, (2) account C1 is converted to multisig and thus invalid
			context.validator().setValidateCosignatoriesResult(
					CosignatoriesValidationResult::Ineligible,
					transaction.CosignaturesPtr()[1].SignerPublicKey);

			// Act:
			auto result = context.updater().update(cosignature).get();

			// Assert: the cosignature was rejected
			EXPECT_EQ(CosignatureUpdateResult::Unverifiable, result);

			const auto* pCosignatures = transaction.CosignaturesPtr();
			context.assertSingleTransactionInCache(transactionInfo.EntityHash, transaction, {
				pCosignatures[0], pCosignatures[2]
			});
			context.assertTransactionInCacheHasCorrectExtendedProperties(transactionInfo);

			EXPECT_TRUE(context.completedTransactions().empty());
			EXPECT_TRUE(context.failedTransactionStatuses().empty());

			ExpectedValidatorCalls expectedValidatorCalls;
			// * 1 (new cosig) + 1 (ineligible) + 3 (existing cosig)
			// * 0 (isComplete) bypassed because the new cosignature is rejected (after stale cosignature pruning)
			expectedValidatorCalls.NumValidateCosignatoriesCalls.setExactMatch(5);
			expectedValidatorCalls.NumLastCosignatories.setExactMatch(1); // 1 (existing cosig)
			context.validator().assertCalls(transaction, expectedValidatorCalls);
		});
	}

	TEST(TEST_CLASS, StaleCosignatureIsNotPurgedWhenNewIneligibleCosignatureIsAdded) {
		// Arrange:
		RunTestWithTransactionInCache(3, [](auto& context, const auto& transactionInfo, const auto& transaction) {
			// - create an ineligible cosignature
			auto cosignature = test::GenerateValidCosignature(transactionInfo.EntityHash);
			context.validator().setValidateCosignatoriesResult(CosignatoriesValidationResult::Ineligible, cosignature.SignerPublicKey);

			// - change an already accepted and valid cosignature to be ineligible
			//   this simulates edge case where: (1) account C1 cosigns, (2) account C1 is converted to multisig and thus invalid
			context.validator().setValidateCosignatoriesResult(
					CosignatoriesValidationResult::Ineligible,
					transaction.CosignaturesPtr()[1].SignerPublicKey);

			// Act:
			auto result = context.updater().update(cosignature).get();

			// Assert: the cosignature was rejected
			EXPECT_EQ(CosignatureUpdateResult::Ineligible, result);

			const auto* pCosignatures = transaction.CosignaturesPtr();
			context.assertSingleTransactionInCache(transactionInfo.EntityHash, transaction, {
				pCosignatures[0], pCosignatures[1], pCosignatures[2]
			});
			context.assertTransactionInCacheHasCorrectExtendedProperties(transactionInfo);

			EXPECT_TRUE(context.completedTransactions().empty());
			EXPECT_TRUE(context.failedTransactionStatuses().empty());

			// - isComplete is bypassed because the new cosignature is rejected (after stale cosignature pruning)
			ExpectedValidatorCalls expectedValidatorCalls;
			expectedValidatorCalls.NumValidateCosignatoriesCalls.setExactMatch(2); // 1 (new cosig) + 1 (ineligible)
			expectedValidatorCalls.NumLastCosignatories.setExactMatch(1); // 1 (new cosig)
			context.validator().assertCalls(transaction, expectedValidatorCalls);
		});
	}

	TEST(TEST_CLASS, StaleCosignatureDoesNotPreventNewCosignatureFromCompletingTransaction) {
		// Arrange:
		RunTestWithTransactionInCache(3, [](auto& context, const auto& transactionInfo, const auto& transaction) {
			// - create a compatible cosignature
			auto cosignature = test::GenerateValidCosignature(transactionInfo.EntityHash);

			// - change an already accepted and valid cosignature to be ineligible (notice that triggers are registered in priority order)
			//   this simulates edge case where: (1) account C1 cosigns, (2) account C1 is converted to multisig and thus invalid
			context.validator().setValidateCosignatoriesResult(
					CosignatoriesValidationResult::Ineligible,
					transaction.CosignaturesPtr()[1].SignerPublicKey);

			// - mark the transaction as complete
			context.validator().setValidateCosignatoriesResult(CosignatoriesValidationResult::Success, cosignature.SignerPublicKey);

			// Act:
			auto result = context.updater().update(cosignature).get();

			// Assert: the cosignature was added
			EXPECT_EQ(CosignatureUpdateResult::Added_Complete, result);

			EXPECT_EQ(0u, context.transactionsCache().view().size());

			const auto* pCosignatures = transaction.CosignaturesPtr();
			ASSERT_EQ(1u, context.completedTransactions().size());
			test::AssertStitchedTransaction(*context.completedTransactions()[0], transaction, {
				pCosignatures[0], pCosignatures[2],
				cosignature
			});
			EXPECT_TRUE(context.failedTransactionStatuses().empty());

			ExpectedValidatorCalls expectedValidatorCalls;
			// * 1 (new cosig) + 1 (ineligible) + 3 (existing cosig) + 1 (isComplete)
			expectedValidatorCalls.NumValidateCosignatoriesCalls.setExactMatch(6);
			expectedValidatorCalls.NumLastCosignatories.setExactMatch(3); // 1 (new cosig) + 2 (existing valid cosig)
			context.validator().assertCalls(transaction, expectedValidatorCalls);
		});
	}

	// endregion

	// region update cosignature - batch adding

	namespace {
		void LogCounts(const std::unordered_map<CosignatureUpdateResult, size_t>& counts) {
			for (const auto& pair : counts)
				CATAPULT_LOG(debug) << "update result " << utils::to_underlying_type(pair.first) << " - " << pair.second;
		}
	}

	TEST(TEST_CLASS, AddingManyDuplicateCosignaturesWithMatchingTransactionOnlyAddsOne) {
		// Arrange:
		RunTestWithTransactionInCache(3, [](auto& context, const auto& transactionInfo, const auto& transaction) {
			// - create a compatible cosignature
			auto cosignature = test::GenerateValidCosignature(transactionInfo.EntityHash);

			// Act: add many duplicates of the same cosignature (make sure there are more adds than threads to queue some)
			auto numAddAttempts = 2 * context.numWorkerThreads();
			std::vector<thread::future<CosignatureUpdateResult>> futures;
			for (auto i = 0u; i < numAddAttempts; ++i)
				futures.push_back(context.updater().update(cosignature));

			// - wait for all adds to finish
			auto results = thread::get_all(std::move(futures));

			// - count the number of results
			std::unordered_map<CosignatureUpdateResult, size_t> counts;
			for (auto result : results)
				++counts[result];

			LogCounts(counts);

			// Assert: only one add should have succeeded
			EXPECT_EQ(2u, counts.size());
			EXPECT_EQ(1u, counts[CosignatureUpdateResult::Added_Incomplete]);
			EXPECT_EQ(numAddAttempts - 1, counts[CosignatureUpdateResult::Redundant]);

			const auto* pCosignatures = transaction.CosignaturesPtr();
			context.assertSingleTransactionInCache(transactionInfo.EntityHash, transaction, {
				pCosignatures[0], pCosignatures[1], pCosignatures[2],
				cosignature
			});
			context.assertTransactionInCacheHasCorrectExtendedProperties(transactionInfo);

			EXPECT_TRUE(context.completedTransactions().empty());
			EXPECT_TRUE(context.failedTransactionStatuses().empty());

			ExpectedValidatorCalls expectedValidatorCalls;
			// * min is 2:
			//   - one cosignature eligible and completed checks (2)
			//   - all others rejected by cache check prior to eligible check (0)
			// * max is numAddAttempts + 1:
			//   - all cosignatures eligible check (numAddAttempts)
			//   - one cosignature completed check (1)
			expectedValidatorCalls.NumValidateCosignatoriesCalls.setInclusiveRangeMatch(2, numAddAttempts + 1);
			expectedValidatorCalls.NumLastCosignatories.setExactMatch(3 + 1);
			context.validator().assertCalls(transaction, expectedValidatorCalls);
		});
	}

	TEST(TEST_CLASS, AddingManyValidCompletingCosignaturesOnlyCompletesTransactionOnce) {
		// Arrange:
		RunTestWithTransactionInCache(3, [](auto& context, const auto& transactionInfo, const auto& transaction) {
			// - always mark the transaction as complete and randomly sleep in validateCosignatories
			const auto* pCosignatures = transaction.CosignaturesPtr();
			context.validator().setValidateCosignatoriesResult(CosignatoriesValidationResult::Success, pCosignatures[0].SignerPublicKey);
			context.validator().sleepInValidateCosignatories();

			// Act: add many different completing cosignatures (make sure there are more adds than threads to queue some)
			auto numAddAttempts = 2 * context.numWorkerThreads();
			std::vector<thread::future<CosignatureUpdateResult>> futures;
			for (auto i = 0u; i < numAddAttempts; ++i)
				futures.push_back(context.updater().update(test::GenerateValidCosignature(transactionInfo.EntityHash)));

			// - wait for all adds to finish
			auto results = thread::get_all(std::move(futures));

			// - count the number of results
			std::unordered_map<CosignatureUpdateResult, size_t> counts;
			for (auto result : results)
				++counts[result];

			LogCounts(counts);

			// Assert: at least one add should have completed
			EXPECT_LE(1u, counts.size());
			EXPECT_LE(1u, counts[CosignatureUpdateResult::Added_Complete]);

			// - the (completed) transaction should have been removed
			EXPECT_EQ(0u, context.transactionsCache().view().size());

			// - ignore the last cosignature(s) attached to the completed transaction because they are a nondeterministic set of candidates
			ASSERT_EQ(1u, context.completedTransactions().size());

			const auto& completedTransaction = *context.completedTransactions()[0];
			std::vector<model::Cosignature> expectedCosignatures{ pCosignatures[0], pCosignatures[1], pCosignatures[2] };
			auto numUnknownCosignatures = (completedTransaction.Size - transaction.Size) / sizeof(model::Cosignature);
			test::AssertStitchedTransaction(completedTransaction, transaction, expectedCosignatures, numUnknownCosignatures);

			EXPECT_TRUE(context.failedTransactionStatuses().empty());

			ExpectedValidatorCalls expectedValidatorCalls;
			// * min is 2:
			//   - one cosignature eligible and completed checks (2)
			//   - all others rejected by cache check prior to eligible check (0)
			// * max is 2 * numAddAttempts:
			//   - all cosignatures eligible check (numAddAttempts)
			//   - all cosignatures completed check (numAddAttempts)
			expectedValidatorCalls.NumValidateCosignatoriesCalls.setInclusiveRangeMatch(2, 2 * numAddAttempts);
			// * min is 4:
			//   - original cosignatures (3)
			//   - one new cosignature (1)
			// * max is 3 + numAddAttempts:
			//   - original cosignatures (3)
			//   - all new cosignatures (numAddAttempts)
			expectedValidatorCalls.NumLastCosignatories.setInclusiveRangeMatch(4, 3 + numAddAttempts);
			context.validator().assertCalls(transaction, expectedValidatorCalls);
		});
	}

	// endregion

	// region threading

	TEST(TEST_CLASS, FuturesAreFulfilledEvenWhenUpdaterIsDestroyed) {
		// Arrange:
		UpdaterTestContext context;
		auto pTransaction = CreateRandomAggregateTransaction(3);
		auto transactionInfo = CreateRandomTransactionInfo(pTransaction);
		test::FixCosignatures(transactionInfo.EntityHash, *pTransaction);

		// Act: start async operations and destroy the updater
		auto future1 = context.updater().update(transactionInfo);
		auto future2 = context.updater().update(test::GenerateValidCosignature(test::GenerateRandomByteArray<Hash256>()));
		context.destroyUpdater();

		// - wait for operations to complete
		auto result1 = future1.get();
		auto result2 = future2.get();

		// Assert:
		EXPECT_EQ_TRANSACTION_UPDATE_RESULT(result1, New, 3);
		EXPECT_EQ(CosignatureUpdateResult::Ineligible, result2);
	}

	// endregion
}}

#include "partialtransaction/src/chain/PtUpdater.h"
#include "partialtransaction/src/chain/PtValidator.h"
#include "plugins/txes/aggregate/src/model/AggregateTransaction.h"
#include "catapult/cache/MemoryPtCache.h"
#include "catapult/model/TransactionStatus.h"
#include "catapult/thread/FutureUtils.h"
#include "catapult/utils/MemoryUtils.h"
#include "catapult/utils/SpinLock.h"
#include "partialtransaction/tests/test/AggregateTransactionTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/other/ValidationResultTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

#define TEST_CLASS PtUpdaterTests

#define EXPECT_EQ_TRANSACTION_UPDATE_RESULT(RESULT, UPDATE_TYPE, NUM_COSIGNATURES_ADDED) \
	EXPECT_EQ(TransactionUpdateResult::UpdateType::UPDATE_TYPE, RESULT.Type); \
	EXPECT_EQ(NUM_COSIGNATURES_ADDED, RESULT.NumCosignaturesAdded);

	namespace {
		model::TransactionInfo CreateRandomTransactionInfo(const std::shared_ptr<const model::Transaction>& pTransaction) {
			auto transactionInfo = test::CreateRandomTransactionInfo();
			transactionInfo.pEntity = pTransaction;
			return transactionInfo;
		}

		std::shared_ptr<model::AggregateTransaction> CreateRandomAggregateTransaction(uint32_t numCosignatures) {
			return test::CreateRandomAggregateTransactionWithCosignatures(numCosignatures);
		}

		// region mocks

		class ValidateCosignersResultTrigger {
		public:
			ValidateCosignersResultTrigger() : ValidateCosignersResultTrigger(0)
			{}

			ValidateCosignersResultTrigger(size_t id) : m_id(id)
			{}

			ValidateCosignersResultTrigger(Key signer)
					: m_id(0) // notice that calls are 1-based, so this will never match
					, m_signer(signer)
			{}

		public:
			bool operator()(const test::CosignaturesMap& cosignaturesMap, size_t id) const {
				return id == m_id || cosignaturesMap.cend() != cosignaturesMap.find(m_signer);
			}

		private:
			size_t m_id;
			Key m_signer;
		};

		constexpr auto Validate_Partial_Raw_Result = test::MakeValidationResult(validators::ResultSeverity::Failure, 12);
		constexpr auto Validate_Cosigners_Raw_Result = test::MakeValidationResult(validators::ResultSeverity::Failure, 24);

		struct ExpectedValidatorCalls {
		private:
			using CountPredicate = predicate<size_t>;

		public:
			ExpectedValidatorCalls(size_t numValidatePartialCalls, size_t numValidateCosignersCalls, size_t numLastCosigners)
					: ExpectedValidatorCalls(numValidatePartialCalls, ExactMatch(numValidateCosignersCalls), numLastCosigners)
			{}

			ExpectedValidatorCalls(
					size_t numValidatePartialCalls,
					const CountPredicate& numValidateCosignersCallsPredicate,
					size_t numLastCosigners)
					: ExpectedValidatorCalls(numValidatePartialCalls, numValidateCosignersCallsPredicate, ExactMatch(numLastCosigners))
			{}

			ExpectedValidatorCalls(
					size_t numValidatePartialCalls,
					size_t numValidateCosignersCalls,
					const CountPredicate& numLastCosignersPredicate)
					: ExpectedValidatorCalls(numValidatePartialCalls, ExactMatch(numValidateCosignersCalls), numLastCosignersPredicate)
			{}

			ExpectedValidatorCalls(
					size_t numValidatePartialCalls,
					const CountPredicate& numValidateCosignersCallsPredicate,
					const CountPredicate& numLastCosignersPredicate)
					: NumValidatePartialCalls(numValidatePartialCalls)
					, NumValidateCosignersCallsPredicate(numValidateCosignersCallsPredicate)
					, NumLastCosignersPredicate(numLastCosignersPredicate)
			{}

		private:
			CountPredicate ExactMatch(size_t expected) {
				return [expected](auto actual) { return expected == actual; };
			}

		public:
			size_t NumValidatePartialCalls;
			CountPredicate NumValidateCosignersCallsPredicate;
			CountPredicate NumLastCosignersPredicate;
		};

		class MockPtValidator : public PtValidator {
		public:
			MockPtValidator()
					: m_validatePartialResult(true)
					, m_validateCosignersResult(CosignersValidationResult::Missing)
					, m_shouldSleepInValidateCosigners(false)
					, m_numValidatePartialCalls(0)
					, m_numValidateCosignersCalls(0)
					, m_numLastCosigners(0)
			{}

		public:
			void setValidatePartialFailure() {
				m_validatePartialResult = false;
			}

			void setValidateCosignersResult(CosignersValidationResult result, const ValidateCosignersResultTrigger& trigger) {
				m_validateCosignersResult = result;
				m_validateCosignersResultTrigger = trigger;
			}

			void sleepInValidateCosigners() {
				m_shouldSleepInValidateCosigners = true;
			}

		public:
			Result<bool> validatePartial(const model::WeakEntityInfoT<model::Transaction>& transactionInfo) const override {
				utils::SpinLockGuard guard(m_lock);
				++m_numValidatePartialCalls;
				m_transactions.push_back(test::CopyTransaction(transactionInfo.entity()));
				m_transactionHashes.push_back(transactionInfo.hash());
				return { Validate_Partial_Raw_Result, m_validatePartialResult };
			}

			Result<CosignersValidationResult> validateCosigners(const model::WeakCosignedTransactionInfo& transactionInfo) const override {
				test::CosignaturesMap cosignaturesMap;
				{
					utils::SpinLockGuard guard(m_lock);
					++m_numValidateCosignersCalls;
					m_transactions.push_back(test::CopyTransaction(transactionInfo.transaction()));

					// there shouldn't be any duplicate cosigners, but filter out duplicates just in case
					cosignaturesMap = test::ToMap(transactionInfo.cosignatures());
					m_numLastCosigners = cosignaturesMap.size();
				}

				if (m_shouldSleepInValidateCosigners) {
					// sleep for a random amount of time to exploit potential race conditions
					auto sleepMs = test::RandomByte() / 2;
					CATAPULT_LOG(debug) << "sleeping for " << static_cast<int>(sleepMs) << "ms";
					test::Sleep(sleepMs);
				}

				auto cosignersValidationResult = m_validateCosignersResultTrigger(cosignaturesMap, m_numValidateCosignersCalls)
						? m_validateCosignersResult
						: CosignersValidationResult::Missing;
				return { Validate_Cosigners_Raw_Result, cosignersValidationResult };
			}

		public:
			void reset() {
				m_numValidatePartialCalls = 0;
				m_numValidateCosignersCalls = 0;
				m_numLastCosigners = 0;
				m_transactions.clear();
				m_transactionHashes.clear();
			}

			void assertCalls(const model::AggregateTransaction& aggregateTransaction, const ExpectedValidatorCalls& expected) {
				// Assert: check calls
				assertCallsImpl(aggregateTransaction, expected);

				// - other overload should be used if there are verify partial calls
				EXPECT_EQ(0u, expected.NumValidatePartialCalls);
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
				EXPECT_NE(0u, expected.NumValidatePartialCalls);
			}

		private:
			void assertCallsImpl(const model::AggregateTransaction& aggregateTransaction, const ExpectedValidatorCalls& expected) {
				// Assert: check calls
				CATAPULT_LOG(debug)
						<< "NumValidatePartialCalls = " << m_numValidatePartialCalls
						<< ", NumValidateCosignersCalls = " << m_numValidateCosignersCalls
						<< ", NumLastCosigners = " << m_numLastCosigners;
				EXPECT_EQ(expected.NumValidatePartialCalls, m_numValidatePartialCalls);
				EXPECT_TRUE(expected.NumValidateCosignersCallsPredicate(m_numValidateCosignersCalls))
						<< "actual " << m_numValidateCosignersCalls;
				EXPECT_TRUE(expected.NumLastCosignersPredicate(m_numLastCosigners))
						<< "actual " << m_numLastCosigners;

				// - check forwarded transactions
				auto pTransactionWithoutCosignatures = test::StripCosignatures(aggregateTransaction);
				for (const auto& pTransaction : m_transactions)
					EXPECT_EQ(*pTransactionWithoutCosignatures, *pTransaction);
			}

		private:
			bool m_validatePartialResult;
			CosignersValidationResult m_validateCosignersResult;
			ValidateCosignersResultTrigger m_validateCosignersResultTrigger;
			bool m_shouldSleepInValidateCosigners;

			mutable utils::SpinLock m_lock;
			mutable size_t m_numValidatePartialCalls;
			mutable size_t m_numValidateCosignersCalls;
			mutable size_t m_numLastCosigners;
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
					, m_pPool(test::CreateStartedIoServiceThreadPool())
					, m_pUpdater(std::make_unique<PtUpdater>(
							m_transactionsCache,
							std::move(m_pUniqueValidator),
							PtUpdater::CompletedTransactionSink([this](auto&& pTransaction) {
								m_completedTransactions.push_back(std::move(pTransaction));
							}),
							[this](const auto& transaction, const auto& hash, auto result) {
								// notice that transaction.Deadline is used as transaction marker
								m_failedTransactionStatuses.emplace_back(hash, utils::to_underlying_type(result), transaction.Deadline);
							},
							m_pPool))
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
			/// Asserts that the pt cache contains a \em single \a aggregateTransaction with \a aggregateHash and \a cosignatures.
			void assertSingleTransactionInCache(
					const Hash256& aggregateHash,
					const model::AggregateTransaction& aggregateTransaction,
					const std::vector<model::Cosignature>& cosignatures) const {
				// Assert:
				auto view = m_transactionsCache.view();
				EXPECT_EQ(1u, view.size());

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
					auto message = "cosigner " + test::ToHexString(cosignature.Signer);

					auto iter = expectedCosignaturesMap.find(cosignature.Signer);
					ASSERT_NE(expectedCosignaturesMap.cend(), iter) << message;
					EXPECT_EQ(iter->first, cosignature.Signer) << message;
					EXPECT_EQ(iter->second, cosignature.Signature) << message;
					expectedCosignaturesMap.erase(iter);
				}

				EXPECT_TRUE(expectedCosignaturesMap.empty());

				// - transaction is stripped of all cosignatures
				auto pTransactionWithoutCosignatures = test::StripCosignatures(aggregateTransaction);
				EXPECT_EQ(*pTransactionWithoutCosignatures, transactionInfoFromCache.transaction());
			}

		public:
			/// Asserts that the transaction cache contains correct extended properties for transaction info (\a transactionInfo).
			/// \note This will modify the cache.
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
			/// Asserts that the failed transaction callback was called with \a expectedResult for transaction info (\a transactionInfo).
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

			std::shared_ptr<thread::IoServiceThreadPool> m_pPool;
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
		EXPECT_EQ_TRANSACTION_UPDATE_RESULT(result, Invalid, 0u);
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
		EXPECT_EQ_TRANSACTION_UPDATE_RESULT(result, New, 0u);
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
		EXPECT_EQ_TRANSACTION_UPDATE_RESULT(result, New, 3u);

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
		context.validator().setValidateCosignersResult(CosignersValidationResult::Success, 1);

		// Act:
		auto result = context.updater().update(transactionInfo).get();

		// Assert:
		EXPECT_EQ_TRANSACTION_UPDATE_RESULT(result, New, 0u);

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
		context.validator().setValidateCosignersResult(CosignersValidationResult::Success, 6);

		// Act:
		auto result = context.updater().update(transactionInfo).get();

		// Assert:
		EXPECT_EQ_TRANSACTION_UPDATE_RESULT(result, New, 3u);

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
			EXPECT_EQ_TRANSACTION_UPDATE_RESULT(result, Existing, 2u);

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
			EXPECT_EQ_TRANSACTION_UPDATE_RESULT(result, Existing, 2u);

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
			context.validator().setValidateCosignersResult(CosignersValidationResult::Success, 4);

			// Act: add a second transaction with same hash
			auto pTransaction2 = CreateRandomAggregateTransaction(2);
			auto transactionInfo2 = CopyAndReplaceTransaction(transactionInfo1, pTransaction2);
			test::FixCosignatures(transactionInfo2.EntityHash, *pTransaction2);
			auto result = context.updater().update(transactionInfo2).get();

			// Assert: the original transaction (for the test the tx data is different) should be used with merged cosignatures
			EXPECT_EQ_TRANSACTION_UPDATE_RESULT(result, Existing, 2u);

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
		void RunTransactionWithInvalidCosignatureTest(TCorruptCosignature corruptCosignature) {
			// Arrange:
			UpdaterTestContext context;
			auto pTransaction = CreateRandomAggregateTransaction(3);
			auto transactionInfo = CreateRandomTransactionInfo(pTransaction);
			test::FixCosignatures(transactionInfo.EntityHash, *pTransaction);

			// - mark a cosigner as invalid
			corruptCosignature(context, pTransaction->CosignaturesPtr()[1]);

			// Act:
			auto result = context.updater().update(transactionInfo).get();

			// Assert: the invalid cosignature should not have been added
			EXPECT_EQ_TRANSACTION_UPDATE_RESULT(result, New, 2u);

			const auto* pCosignatures = pTransaction->CosignaturesPtr();
			context.assertSingleTransactionInCache(transactionInfo.EntityHash, *pTransaction, { pCosignatures[0], pCosignatures[2] });
			context.assertTransactionInCacheHasCorrectExtendedProperties(transactionInfo);

			EXPECT_TRUE(context.completedTransactions().empty());
			EXPECT_TRUE(context.failedTransactionStatuses().empty());

			auto numLastCosignersPredicate = [](auto actual) {
				// 2: { Valid, Invalid, Valid }, { Invalid, Valid, Valid } - invalid cosignature is excluded from subsequent calls
				// 3: { Valid, Valid, Invalid } - last call includes two valid and one invalid cosignatures
				return 2 <= actual && actual <= 3;
			};
			context.validator().assertCalls(*pTransaction, transactionInfo.EntityHash, { 1, 5, numLastCosignersPredicate });
		}
	}

	TEST(TEST_CLASS, AddingAggregateWithCosignaturesIgnoresIneligibleCosignatures) {
		// Arrange:
		RunTransactionWithInvalidCosignatureTest([](auto& context, const auto& cosignature) {
			// - mark a cosigner as ineligible
			context.validator().setValidateCosignersResult(CosignersValidationResult::Ineligible, cosignature.Signer);
		});
	}

	TEST(TEST_CLASS, AddingAggregateWithCosignaturesIgnoresUnverifiableCosignatures) {
		// Arrange:
		RunTransactionWithInvalidCosignatureTest([](const auto&, auto& cosignature) {
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
		EXPECT_EQ_TRANSACTION_UPDATE_RESULT(result, New, 2u);

		const auto* pCosignatures = pTransaction->CosignaturesPtr();
		context.assertSingleTransactionInCache(transactionInfo.EntityHash, *pTransaction, { pCosignatures[0], pCosignatures[1] });
		context.assertTransactionInCacheHasCorrectExtendedProperties(transactionInfo);

		EXPECT_TRUE(context.completedTransactions().empty());
		EXPECT_TRUE(context.failedTransactionStatuses().empty());
		context.validator().assertCalls(*pTransaction, transactionInfo.EntityHash, { 1, 4, 2 });
	}

	// endregion

	// region update cosignature - ignored

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
			context.validator().setValidateCosignersResult(CosignersValidationResult::Success, 2);

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
			context.validator().setValidateCosignersResult(CosignersValidationResult::Failure, 1);

			// Act:
			auto result = context.updater().update(cosignature).get();

			// Assert: the cosignature triggered a failure
			EXPECT_EQ(CosignatureUpdateResult::Ineligible, result);

			// - the transaction was purged from the cache
			EXPECT_EQ(0u, context.transactionsCache().view().size());

			EXPECT_TRUE(context.completedTransactions().empty());
			context.assertSingleFailedTransaction(transactionInfo, Validate_Cosigners_Raw_Result);
			context.validator().assertCalls(transaction, { 0, 1, 3 + 1 });
		});
	}

	namespace {
		template<typename TCorruptCosignature>
		void RunAddingInvalidCosignatureTest(CosignatureUpdateResult expectedResult, TCorruptCosignature corruptCosignature) {
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
					pCosignatures[0], pCosignatures[1], pCosignatures[2],
				});
				context.assertTransactionInCacheHasCorrectExtendedProperties(transactionInfo);

				EXPECT_TRUE(context.completedTransactions().empty());
				EXPECT_TRUE(context.failedTransactionStatuses().empty());
				context.validator().assertCalls(transaction, { 0, 1, 3 + 1 });
			});
		}
	}

	TEST(TEST_CLASS, AddingIneligibleCosignatureWithMatchingTransactionIsIgnored) {
		// Arrange:
		RunAddingInvalidCosignatureTest(CosignatureUpdateResult::Ineligible, [](auto& context, const auto&) {
			// - mark the cosignature as ineligible
			context.validator().setValidateCosignersResult(CosignersValidationResult::Ineligible, 1);
		});
	}

	TEST(TEST_CLASS, AddingUnverifiableCosignatureWithMatchingTransactionIsIgnored) {
		// Arrange:
		RunAddingInvalidCosignatureTest(CosignatureUpdateResult::Unverifiable, [](const auto&, auto& cosignature) {
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
				existingCosignature.Signer,
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
			auto numValidateCosignersCallsPredicate = [numAddAttempts](auto actual) {
				// min is 2:
				// - one cosignature eligible and completed checks (2)
				// - all others rejected by cache check prior to eligible check (0)
				// max is numAddAttempts + 1:
				// - all cosignatures eligible check (numAddAttempts)
				// - one cosignature completed check (1)
				return 2 <= actual && actual <= numAddAttempts + 1;
			};
			context.validator().assertCalls(transaction, { 0, numValidateCosignersCallsPredicate, 3 + 1 });
		});
	}

	TEST(TEST_CLASS, AddingManyValidCompletingCosignaturesOnlyCompletesTransactionOnce) {
		// Arrange:
		RunTestWithTransactionInCache(3, [](auto& context, const auto& transactionInfo, const auto& transaction) {
			// - always mark the transaction as complete and randomly sleep in validateCosigners
			const auto* pCosignatures = transaction.CosignaturesPtr();
			context.validator().setValidateCosignersResult(CosignersValidationResult::Success, pCosignatures[0].Signer);
			context.validator().sleepInValidateCosigners();

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
			std::vector<model::Cosignature> expectedCosignatures = { pCosignatures[0], pCosignatures[1], pCosignatures[2] };
			auto numUnknownCosignatures = (completedTransaction.Size - transaction.Size) / sizeof(model::Cosignature);
			test::AssertStitchedTransaction(completedTransaction, transaction, expectedCosignatures, numUnknownCosignatures);

			EXPECT_TRUE(context.failedTransactionStatuses().empty());
			auto numValidateCosignersCallsPredicate = [numAddAttempts](auto actual) {
				// min is 2:
				// - one cosignature eligible and completed checks (2)
				// - all others rejected by cache check prior to eligible check (0)
				// max is 2 * numAddAttempts:
				// - all cosignatures eligible check (numAddAttempts)
				// - all cosignatures completed check (numAddAttempts)
				return 2 <= actual && actual <= 2 * numAddAttempts;
			};
			auto numLastCosignersPredicate = [numAddAttempts](auto actual) {
				// min is 4:
				// - original cosignatures (3)
				// - one new cosignature (1)
				// max is 3 + numAddAttempts:
				// - original cosignatures (3)
				// - all new cosignatures (numAddAttempts)
				return 4 <= actual && actual <= 3 + numAddAttempts;
			};
			context.validator().assertCalls(transaction, { 0, numValidateCosignersCallsPredicate, numLastCosignersPredicate });
		});
	}

	// endregion

	// region threading

	TEST(TEST_CLASS, FuturesAreFulfilledEvenIfUpdaterIsDestroyed) {
		// Arrange:
		UpdaterTestContext context;
		auto pTransaction = CreateRandomAggregateTransaction(3);
		auto transactionInfo = CreateRandomTransactionInfo(pTransaction);
		test::FixCosignatures(transactionInfo.EntityHash, *pTransaction);

		// Act: start async operations and destroy the updater
		auto future1 = context.updater().update(transactionInfo);
		auto future2 = context.updater().update(test::GenerateValidCosignature(test::GenerateRandomData<Hash256_Size>()));
		context.destroyUpdater();

		// - wait for operations to complete
		auto result1 = future1.get();
		auto result2 = future2.get();

		// Assert:
		EXPECT_EQ_TRANSACTION_UPDATE_RESULT(result1, New, 3u);
		EXPECT_EQ(CosignatureUpdateResult::Ineligible, result2);
	}

	// endregion
}}

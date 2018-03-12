#include "catapult/chain/UtUpdater.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/model/TransactionStatus.h"
#include "tests/catapult/chain/test/MockExecutionConfiguration.h"
#include "tests/test/cache/UtTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/TestHarness.h"

using catapult::validators::ValidationResult;

namespace catapult { namespace chain {

#define TEST_CLASS UtUpdaterTests

	namespace {
		constexpr auto Default_Height = Height(17);
		constexpr auto Default_Time = Timestamp(987);

		ValidationResult Modify(ValidationResult result) {
			// used to modify a ValidationResult while preserving its severity
			return static_cast<ValidationResult>(utils::to_underlying_type(result) ^ 0xFF);
		}

		using IndexResultPairs = std::vector<std::pair<size_t, ValidationResult>>;

		IndexResultPairs GetFailedIndexes(ValidationResult baseResult, const IndexResultPairs& failedIndexes) {
			// only failure results should be raised by the updater
			return validators::IsValidationResultFailure(baseResult) ? failedIndexes : IndexResultPairs();
		}

		auto CreateCacheWithDefaultHeight() {
			auto cache = test::CreateCatapultCacheWithMarkerAccount();
			auto delta = cache.createDelta();
			cache.commit(Default_Height);
			return cache;
		}

		auto ConcatIds(const std::vector<size_t>& lhs, const std::vector<size_t>& rhs, size_t rhsOffset) {
			std::vector<size_t> result;
			for (const auto& item : lhs)
				result.push_back(item);

			for (const auto& item : rhs)
				result.push_back(item + rhsOffset);

			return result;
		}

		class UpdaterTestContext {
		public:
			UpdaterTestContext()
					: m_cache(CreateCacheWithDefaultHeight())
					, m_transactionsCache(cache::MemoryCacheOptions(1024, 1000))
					, m_updater(
							m_transactionsCache,
							m_cache,
							m_executionConfig.Config,
							[]() { return Default_Time; },
							[this](const auto& transaction, const auto& hash, auto result) {
								// notice that transaction.Deadline is used as transaction marker
								m_failedTransactionStatuses.emplace_back(hash, utils::to_underlying_type(result), transaction.Deadline);
							})
			{}

		public:
			cache::MemoryUtCache& transactionsCache() {
				return m_transactionsCache;
			}

			UtUpdater& updater() {
				return m_updater;
			}

			void setValidationResult(ValidationResult result, const Hash256& hash, size_t id) {
				m_executionConfig.pValidator->setResult(result, hash, id);
			}

			void setPartialUndoFailureIndexes(const std::unordered_set<size_t>& partialUndoFailureIndexes) {
				m_partialUndoFailureIndexes = partialUndoFailureIndexes;
			}

		private:
			bool isRollbackExecution(size_t index) const {
				// MockExecutionConfiguration is configured to create two notifications for each entity
				// as such, there are three possible states for each entity:
				// 1. both validations pass => both notifications are executed
				// 2. first validation fails => no notifications are executed
				// 3. second validation fails => first notification is executed and then undone
				//
				// a test sets m_partialUndoFailureIndexes to indicate rollbacks of the first notification in state (3)
				return m_partialUndoFailureIndexes.cend() != m_partialUndoFailureIndexes.find(index);
			}

		public:
			void seedDifficultyInfos(size_t count) {
				auto delta = m_cache.createDelta();
				auto& blockDifficultyCache = delta.sub<cache::BlockDifficultyCache>();
				for (auto i = 0u; i < count; ++i)
					blockDifficultyCache.insert(state::BlockDifficultyInfo(Height(blockDifficultyCache.size() + 1)));

				m_cache.commit(Default_Height);
			}

		private:
			void assertValidatorContexts(const std::vector<size_t>& expectedNumDifficultyInfos) const {
				CATAPULT_LOG(debug) << "checking validator contexts passed to validator";
				size_t i = 0;
				ASSERT_EQ(expectedNumDifficultyInfos.size(), m_executionConfig.pValidator->params().size());
				for (const auto& params : m_executionConfig.pValidator->params()) {
					auto message = "validator at " + std::to_string(i);
					// - context
					EXPECT_EQ(Default_Height + Height(1), params.Context.Height) << message;
					EXPECT_EQ(Default_Time, params.Context.BlockTime) << message;
					EXPECT_EQ(test::Mock_Execution_Configuration_Network_Identifier, params.Context.Network.Identifier) << message;

					// - cache contents + sequence (NumDifficultyInfos is incremented by each observer call)
					EXPECT_TRUE(params.IsPassedMarkedCache) << message;
					EXPECT_EQ(expectedNumDifficultyInfos[i], params.NumDifficultyInfos) << message;
					++i;
				}
			}

			void assertObserverContexts(size_t numInitialCacheDifficultyInfos) const {
				CATAPULT_LOG(debug) << "checking observer contexts passed to observer";
				size_t i = 0;
				for (const auto& params : m_executionConfig.pObserver->params()) {
					auto message = "observer at " + std::to_string(i);
					// - context
					EXPECT_EQ(Default_Height + Height(1), params.Context.Height) << message;
					if (isRollbackExecution(i))
						EXPECT_EQ(observers::NotifyMode::Rollback, params.Context.Mode) << message;
					else
						EXPECT_EQ(observers::NotifyMode::Commit, params.Context.Mode) << message;

					// - compare the copied state to the default state
					//   (a dummy state is passed by the updater because only block observers modify it)
					EXPECT_EQ(model::ImportanceHeight(0), params.StateCopy.LastRecalculationHeight) << message;

					// - cache contents + sequence (NumDifficultyInfos is incremented by each observer call)
					EXPECT_TRUE(params.IsPassedMarkedCache) << message;
					EXPECT_EQ(numInitialCacheDifficultyInfos + i, params.NumDifficultyInfos) << message;
					++i;
				}
			}

		public:
			void assertContexts(const std::vector<size_t>& expectedNumDifficultyInfos, size_t numInitialCacheDifficultyInfos = 0) const {
				// Assert:
				assertValidatorContexts(expectedNumDifficultyInfos);
				assertObserverContexts(numInitialCacheDifficultyInfos);
			}

			void assertContexts(size_t numInitialCacheDifficultyInfos = 0) const {
				// Assert: assume no observers were skipped (each observer adds a difficulty info to the cache)
				std::vector<size_t> expectedNumDifficultyInfos;
				for (auto i = 0u; i < m_executionConfig.pObserver->params().size(); ++i)
					expectedNumDifficultyInfos.push_back(numInitialCacheDifficultyInfos + i);

				assertContexts(expectedNumDifficultyInfos, numInitialCacheDifficultyInfos);
			}

		private:
			void assertPublisherEntityInfos(const model::WeakEntityInfos& entityInfos) const {
				// Assert: publisher should be called for each entity
				CATAPULT_LOG(debug) << "checking entities passed to publisher";
				const auto& publisherParams = m_executionConfig.pNotificationPublisher->params();
				for (auto i = 0u; i < publisherParams.size(); ++i) {
					const auto& expectedEntityInfo = entityInfos[i];
					const auto message = "publisher at " + std::to_string(i);

					// - entity pointer should be unchanged, hash should have been copied
					EXPECT_EQ(&expectedEntityInfo.entity(), &publisherParams[i].EntityInfo.entity()) << message;
					EXPECT_EQ(expectedEntityInfo.hash(), publisherParams[i].HashCopy) << message;
				}
			}

			void assertValidatorEntityInfos(const model::WeakEntityInfos& entityInfos, const std::vector<size_t>& expectedIndexes) const {
				// Assert:
				CATAPULT_LOG(debug) << "checking entities passed to validator";
				const auto& validatorParams = m_executionConfig.pValidator->params();
				ASSERT_EQ(expectedIndexes.size(), validatorParams.size());

				for (auto i = 0u; i < validatorParams.size(); ++i) {
					auto entityIndex = expectedIndexes[i];
					const auto message = "validator at expected index " + std::to_string(i);

					// - validator params
					EXPECT_EQ(entityInfos[entityIndex].hash(), validatorParams[i].HashCopy) << message;
					EXPECT_EQ(i > 0 && entityIndex == expectedIndexes[i - 1] ? 2u : 1u, validatorParams[i].SequenceId) << message;
				}
			}

			void assertObserverEntityInfos(const model::WeakEntityInfos& entityInfos, const std::vector<size_t>& expectedIndexes) const {
				// Assert:
				CATAPULT_LOG(debug) << "checking entities passed to observer";
				const auto& observerParams = m_executionConfig.pObserver->params();
				ASSERT_EQ(expectedIndexes.size(), observerParams.size());

				for (auto i = 0u; i < observerParams.size(); ++i) {
					auto entityIndex = expectedIndexes[i];
					const auto message = "observer at expected index " + std::to_string(i);

					// - observer params
					EXPECT_EQ(entityInfos[entityIndex].hash(), observerParams[i].HashCopy) << message;
					if (isRollbackExecution(i)) {
						// the rollback id is always 1 because at most one notification per entity can be rolled back
						EXPECT_EQ(1u, observerParams[i].SequenceId) << message;
					} else {
						EXPECT_EQ(i > 0 && entityIndex == expectedIndexes[i - 1] ? 2u : 1u, observerParams[i].SequenceId) << message;
					}
				}
			}

		public:
			void assertEntityInfos(const model::WeakEntityInfos& entityInfos, const IndexResultPairs& failedIndexes = {}) const {
				// Assert: publisher, validator and observer were called with same entity infos
				assertEntityInfos(entityInfos, entityInfos, entityInfos, failedIndexes);
			}

			void assertEntityInfos(
					const model::WeakEntityInfos& publisherEntityInfos,
					const model::WeakEntityInfos& validatorEntityInfos,
					const model::WeakEntityInfos& observerEntityInfos,
					const IndexResultPairs& failedIndexes = {}) const {
				// Arrange:
				std::vector<size_t> validatorIndexes;
				for (auto i = 0u; i < validatorEntityInfos.size(); ++i) {
					validatorIndexes.push_back(i);
					validatorIndexes.push_back(i);
				}

				std::vector<size_t> observerIndexes;
				for (auto i = 0u; i < observerEntityInfos.size(); ++i) {
					observerIndexes.push_back(i);
					observerIndexes.push_back(i);
				}

				// Assert: publisher, validator and observer were called with expected entity infos
				assertEntityInfos(publisherEntityInfos, validatorIndexes, observerIndexes, failedIndexes);
			}

			void assertEntityInfos(
					const model::WeakEntityInfos& publisherEntityInfos,
					const std::vector<size_t>& validatorIndexes,
					const std::vector<size_t>& observerIndexes,
					const IndexResultPairs& failedIndexes = {}) const {
				// Assert: publisher, validator and observer were called with expected entity infos
				assertPublisherEntityInfos(publisherEntityInfos);
				assertValidatorEntityInfos(publisherEntityInfos, validatorIndexes);
				assertObserverEntityInfos(publisherEntityInfos, observerIndexes);

				// - check that transaction failures were raised
				ASSERT_EQ(failedIndexes.size(), m_failedTransactionStatuses.size());
				for (auto i = 0u; i < failedIndexes.size(); ++i) {
					auto failedIndex = failedIndexes[i].first;
					auto message = "failed index " + std::to_string(failedIndex);
					EXPECT_EQ(publisherEntityInfos[failedIndex].hash(), m_failedTransactionStatuses[i].Hash) << message;
					EXPECT_EQ(failedIndexes[i].second, validators::ValidationResult(m_failedTransactionStatuses[i].Status)) << message;
					EXPECT_EQ(Timestamp(failedIndex * failedIndex), m_failedTransactionStatuses[i].Deadline) << message;
				}
			}

		private:
			test::MockExecutionConfiguration m_executionConfig;
			cache::CatapultCache m_cache;
			cache::MemoryUtCache m_transactionsCache;
			UtUpdater m_updater;

			std::unordered_set<size_t> m_partialUndoFailureIndexes;
			std::vector<model::TransactionStatus> m_failedTransactionStatuses;
		};

		template<typename TContainer>
		auto ConcatContainers(const TContainer& lhs, const TContainer& rhs) {
			TContainer result;
			for (const auto& item : lhs)
				result.push_back(item);

			for (const auto& item : rhs)
				result.push_back(item);

			return result;
		}

		template<typename TContainer>
		auto Select(const TContainer& container, const std::vector<size_t>& indexes) {
			TContainer result;
			for (const auto& index : indexes)
				result.push_back(container[index]);

			return result;
		}

		struct TransactionData {
			std::vector<model::TransactionInfo> UtInfos;
			std::vector<const model::VerifiableEntity*> Entities;
			std::vector<Hash256> Hashes;
			std::vector<model::WeakEntityInfo> EntityInfos;
		};

		model::WeakEntityInfo CreateEntityInfoAt(const TransactionData& data, size_t index) {
			return model::WeakEntityInfo(*data.Entities[index], data.UtInfos[index].EntityHash);
		}

		TransactionData CreateTransactionData(size_t count, size_t start = 0) {
			TransactionData data;
			data.UtInfos = test::CreateTransactionInfos(count, [start](auto i) { return Timestamp((i + start) * (i + start)); });
			data.Entities = test::ExtractEntities(data.UtInfos);
			data.Hashes = test::ExtractHashes(data.UtInfos);
			for (auto i = 0u; i < count; ++i)
				data.EntityInfos.push_back(CreateEntityInfoAt(data, i));

			return data;
		}
	}

#define NON_SUCCESS_VALIDATION_TRAITS_BASED_TEST(TEST_NAME) \
	template<ValidationResult TResult> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Neutral) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ValidationResult::Neutral>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Failure) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ValidationResult::Failure>(); } \
	template<ValidationResult TResult> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	namespace {
		struct NewTransactionsTraits {
			static void Update(UtUpdater& updater, const std::vector<model::TransactionInfo>& transactionInfos) {
				updater.update(transactionInfos);
			}
		};

		struct RevertedTransactionsTraits {
			static void Update(UtUpdater& updater, const std::vector<model::TransactionInfo>& transactionInfos) {
				updater.update({}, transactionInfos);
			}
		};
	}

#define NEW_TRANSACTIONS_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_NewTransactions) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NewTransactionsTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_RevertedTransactions) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RevertedTransactionsTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

#define NEW_TRANSACTIONS_NON_SUCCESS_VALIDATION_TRAITS_BASED_TEST(TEST_NAME) \
	NEW_TRANSACTIONS_TRAITS_BASED_TEST(TEST_NAME##_Neutral) { Assert##TEST_NAME<TTraits>(ValidationResult::Neutral); } \
	NEW_TRANSACTIONS_TRAITS_BASED_TEST(TEST_NAME##_Failure) { Assert##TEST_NAME<TTraits>(ValidationResult::Failure); }

	// region shared tests - apply new transactions to cache

	namespace {
		template<typename TTraits>
		void AssertCanApplyNewTransactionsToCache(size_t numTransactions) {
			// Arrange:
			UpdaterTestContext context;
			auto transactionData = CreateTransactionData(numTransactions);

			// Sanity:
			EXPECT_EQ(0u, context.transactionsCache().view().size());

			// Act:
			TTraits::Update(context.updater(), transactionData.UtInfos);

			// Assert:
			EXPECT_EQ(numTransactions, context.transactionsCache().view().size());
			test::AssertContainsAll(context.transactionsCache(), transactionData.Hashes);

			context.assertContexts();
			context.assertEntityInfos(transactionData.EntityInfos);
		}
	}

	NEW_TRANSACTIONS_TRAITS_BASED_TEST(CanApplyZeroNewTransactionsToCache) {
		// Assert:
		AssertCanApplyNewTransactionsToCache<TTraits>(0);
	}

	NEW_TRANSACTIONS_TRAITS_BASED_TEST(CanApplySingleNewTransactionToCache) {
		// Assert:
		AssertCanApplyNewTransactionsToCache<TTraits>(1);
	}

	NEW_TRANSACTIONS_TRAITS_BASED_TEST(CanApplyMultipleTransactionsToCache) {
		// Assert:
		AssertCanApplyNewTransactionsToCache<TTraits>(5);
	}

	// endregion

	// region shared tests - new transactions that fail validation do not get added / are undone

	namespace {
		template<typename TTraits>
		void AssertNewTransactionsThatFailValidationDoNotGetAddedToCache(ValidationResult result) {
			// Arrange:
			UpdaterTestContext context;
			auto transactionData = CreateTransactionData(6);

			// - set failures for 2 / 6 entities
			context.setValidationResult(result, transactionData.Hashes[1], 1);
			context.setValidationResult(Modify(result), transactionData.Hashes[4], 1);

			// Sanity:
			EXPECT_EQ(0u, context.transactionsCache().view().size());

			// Act:
			TTraits::Update(context.updater(), transactionData.UtInfos);

			// Assert:
			EXPECT_EQ(4u, context.transactionsCache().view().size());
			test::AssertContainsAll(context.transactionsCache(), Select(transactionData.Hashes, { 0, 2, 3, 5 }));

			// - observer only gets called for entities that pass validation
			//   E[0] V0,O1,V1,O2; E[1] V2; E[2] V2,O3,V3,O4; E[3] V4,O5,V5,O6; E[4] V6; E[5] V6,O7,V7,O8
			context.assertContexts({ 0, 1, 2, 2, 3, 4, 5, 6, 6, 7 });
			context.assertEntityInfos(
					transactionData.EntityInfos,
					{ 0, 0, 1, 2, 2, 3, 3, 4, 5, 5 },
					{ 0, 0, 2, 2, 3, 3, 5, 5 },
					GetFailedIndexes(result, { { 1, result }, { 4, Modify(result) } }));
		}
	}

	NEW_TRANSACTIONS_NON_SUCCESS_VALIDATION_TRAITS_BASED_TEST(NewTransactionsThatFailValidationDoNotGetAddedToCache);

	namespace {
		template<typename TTraits>
		void AssertNewTransactionsThatPartiallyFailValidationAreUndone(ValidationResult result) {
			// Arrange:
			UpdaterTestContext context;
			auto transactionData = CreateTransactionData(6);

			// - set failures for 2 / 6 entities so that one notification from the entity is observed
			context.setValidationResult(Modify(result), transactionData.Hashes[1], 2);
			context.setValidationResult(result, transactionData.Hashes[4], 2);

			// Sanity:
			EXPECT_EQ(0u, context.transactionsCache().view().size());

			// Act:
			TTraits::Update(context.updater(), transactionData.UtInfos);

			// Assert:
			EXPECT_EQ(4u, context.transactionsCache().view().size());
			test::AssertContainsAll(context.transactionsCache(), Select(transactionData.Hashes, { 0, 2, 3, 5 }));

			// - observer (commit) only gets called for entities that pass validation
			// - observer (rollback) gets called for entities that fail validation
			//   E[0] V0,O1,V1,O2; E[1] V2,O3,V3;RO4 E[2] V4,O5,V5,O6; E[3] V6,O7,V7,O8; E[4] V8,O9,V9;RO10 E[5] V10,O11,V11,O12
			context.setPartialUndoFailureIndexes({ 3, 9 });
			context.assertContexts();
			context.assertEntityInfos(transactionData.EntityInfos, GetFailedIndexes(result, { { 1, Modify(result) }, { 4, result } }));
		}
	}

	NEW_TRANSACTIONS_NON_SUCCESS_VALIDATION_TRAITS_BASED_TEST(NewTransactionsThatPartiallyFailValidationAreUndone);

	// endregion

	// region non empty cache - update with new / reverted transactions

	namespace {
		using AssertContextFunc = consumer<const UpdaterTestContext&, const model::WeakEntityInfos&, const model::WeakEntityInfos&>;

		template<typename TTraits>
		void AssertCanUpdateNonEmptyCacheWithMultipleTransactions(const AssertContextFunc& assertContext) {
			// Arrange: initialize the UT cache with 3 transactions
			UpdaterTestContext context;
			auto originalTransactionData = CreateTransactionData(3);
			test::AddAll(context.transactionsCache(), originalTransactionData.UtInfos);

			// - prepare 4 new transactions
			auto transactionData = CreateTransactionData(4);

			// Sanity:
			EXPECT_EQ(3u, context.transactionsCache().view().size());

			// Act:
			TTraits::Update(context.updater(), transactionData.UtInfos);

			// Assert: the cache contains original and new transactions
			EXPECT_EQ(7u, context.transactionsCache().view().size());
			test::AssertContainsAll(context.transactionsCache(), originalTransactionData.Hashes);
			test::AssertContainsAll(context.transactionsCache(), transactionData.Hashes);

			// - check the context
			assertContext(context, originalTransactionData.EntityInfos, transactionData.EntityInfos);
		}
	}

	TEST(TEST_CLASS, CanUpdateNonEmptyCacheWithMultipleNewTransactions) {
		// Assert:
		AssertCanUpdateNonEmptyCacheWithMultipleTransactions<NewTransactionsTraits>(
				[](const auto& context, const auto&, const auto& entityInfos) {
			// - only new entities were executed
			context.assertContexts();
			context.assertEntityInfos(entityInfos);
		});
	}

	TEST(TEST_CLASS, CanUpdateNonEmptyCacheWithMultipleRevertedTransactions) {
		// Assert:
		AssertCanUpdateNonEmptyCacheWithMultipleTransactions<RevertedTransactionsTraits>(
				[](const auto& context, const auto& originalEntityInfos, const auto& entityInfos) {
			// - both new and original entities were executed
			context.assertContexts();
			context.assertEntityInfos(ConcatContainers(entityInfos, originalEntityInfos));
		});
	}

	namespace {
		void SetAt(TransactionData& data, const model::TransactionInfo& transactionInfo, size_t index) {
			data.UtInfos[index] = transactionInfo.copy();
			data.Entities[index] = transactionInfo.pEntity.get();
			data.Hashes[index] = transactionInfo.EntityHash;
			data.EntityInfos[index] = CreateEntityInfoAt(data, index);
		}

		template<typename TTraits>
		void AssertTransactionsAlreadyInCacheDoNotGetExecuted(const AssertContextFunc& assertContext) {
			// Arrange: initialize the UT cache with 3 transactions
			UpdaterTestContext context;
			auto originalTransactionData = CreateTransactionData(3);

			// - create 5 transactions, including 2 that have already been seen
			auto transactionData = CreateTransactionData(5);
			SetAt(transactionData, originalTransactionData.UtInfos[2], 1);
			SetAt(transactionData, originalTransactionData.UtInfos[0], 3);

			test::AddAll(context.transactionsCache(), originalTransactionData.UtInfos);

			// Sanity:
			EXPECT_EQ(3u, context.transactionsCache().view().size());

			// Act:
			TTraits::Update(context.updater(), transactionData.UtInfos);

			// Assert: the duplicate transactions were not added
			EXPECT_EQ(6u, context.transactionsCache().view().size());
			test::AssertContainsAll(context.transactionsCache(), originalTransactionData.Hashes);
			test::AssertContainsAll(context.transactionsCache(), transactionData.Hashes);

			// - check the context
			assertContext(context, originalTransactionData.EntityInfos, transactionData.EntityInfos);
		}
	}

	TEST(TEST_CLASS, NewTransactionsThatAreAlreadyInCacheDoNotGetExecuted) {
		// Assert:
		AssertTransactionsAlreadyInCacheDoNotGetExecuted<NewTransactionsTraits>(
				[](const auto& context, const auto&, const auto& entityInfos) {
			// - observer only gets called for (unique) entities that were added
			//   E[0] V0,O1,V1,O2; E[1]; E[2] V2,O3,V3,O4; E[3]; E[4] V4,O5,V5,O6
			context.assertContexts({ 0, 1, 2, 3, 4, 5 });

			// - notice that cache check is FIRST so even publishing is short-circuited
			context.assertEntityInfos(Select(entityInfos, { 0, 2, 4 }));
		});
	}

	TEST(TEST_CLASS, RevertedTransactionsThatAreAlreadyInCacheDoNotGetExecuted) {
		// Assert:
		AssertTransactionsAlreadyInCacheDoNotGetExecuted<RevertedTransactionsTraits>(
				[](const auto& context, const auto& originalEntityInfos, const auto& entityInfos) {
			// - observer only gets called for (unique) entities that were added
			//   new: E[0] V0,O1,V1,O2; E[1] V2,O3,V3,O4; E[2] V4,O5,V5,O6; E[3] V6,O7,V7,O8; E[4] V8,O9,V9,O10
			//   old: E[0]; E[1] V0,O1,V1,O2; E[2]
			context.assertContexts(ConcatIds({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }, { 0, 1 }, 10));

			// - notice that cache check is FIRST so even publishing is short-circuited
			context.assertEntityInfos(ConcatContainers(entityInfos, Select(originalEntityInfos, { 1 })));
		});
	}

	// endregion

	// region update (tx disruptor)

	TEST(TEST_CLASS, NewTransactionsValidationIsSkippedIfUnconfirmedCacheIsStale) {
		// Arrange: initialize the UT cache with 3 transactions
		UpdaterTestContext context;
		auto originalTransactionData = CreateTransactionData(3);
		test::AddAll(context.transactionsCache(), originalTransactionData.UtInfos);

		// - modify the catapult cache after creating the updater
		context.seedDifficultyInfos(7);

		// - prepare 4 new transactions
		auto transactionData = CreateTransactionData(4);

		// Sanity:
		EXPECT_EQ(3u, context.transactionsCache().view().size());

		// Act:
		context.updater().update(transactionData.UtInfos);

		// Assert: the cache contains original and new transactions
		EXPECT_EQ(7u, context.transactionsCache().view().size());
		test::AssertContainsAll(context.transactionsCache(), originalTransactionData.Hashes);
		test::AssertContainsAll(context.transactionsCache(), transactionData.Hashes);

		// - neither the validator nor observer were passed any entities
		context.assertContexts();
		context.assertEntityInfos({});
	}

	// endregion

	// region update (block disruptor)

	TEST(TEST_CLASS, RevertedTransactionsUpdateRebasesUnconfirmedCache) {
		// Arrange: initialize the UT cache with 3 transactions
		UpdaterTestContext context;
		auto originalTransactionData = CreateTransactionData(3);
		test::AddAll(context.transactionsCache(), originalTransactionData.UtInfos);

		// - modify the catapult cache after creating the updater
		context.seedDifficultyInfos(7);

		// - prepare 4 new transactions
		auto transactionData = CreateTransactionData(4);

		// Sanity:
		EXPECT_EQ(3u, context.transactionsCache().view().size());

		// Act:
		context.updater().update({}, transactionData.UtInfos);

		// Assert: the cache contains original and new transactions
		EXPECT_EQ(7u, context.transactionsCache().view().size());
		test::AssertContainsAll(context.transactionsCache(), originalTransactionData.Hashes);
		test::AssertContainsAll(context.transactionsCache(), transactionData.Hashes);

		// - both new and original entities were executed relative to the updated cache
		//   (the rebase is implicitly checked by asserting that the first validator and observer were passed
		//    a cache with 7 - instead of 0 - block difficulty infos)
		context.assertContexts(7);
		context.assertEntityInfos(ConcatContainers(transactionData.EntityInfos, originalTransactionData.EntityInfos));
	}

	NON_SUCCESS_VALIDATION_TRAITS_BASED_TEST(OriginalTransactionsThatFailValidationDoNotGetAddedToCache) {
		// Arrange: initialize the UT cache with 6 transactions
		UpdaterTestContext context;

		// - note that the original transactions have timestamps following the new transaction timestamps below
		//   because the new transactions will be processed first
		auto originalTransactionData = CreateTransactionData(6, 3);
		const auto& originalHashes = originalTransactionData.Hashes;
		test::AddAll(context.transactionsCache(), originalTransactionData.UtInfos);

		// - prepare 3 new transactions
		auto transactionData = CreateTransactionData(3);

		// - set failures for 2 / 6 original entities
		context.setValidationResult(TResult, originalHashes[1], 1);
		context.setValidationResult(Modify(TResult), originalHashes[4], 1);

		// Sanity:
		EXPECT_EQ(6u, context.transactionsCache().view().size());

		// Act:
		context.updater().update({}, transactionData.UtInfos);

		// Assert:
		EXPECT_EQ(7u, context.transactionsCache().view().size());
		test::AssertContainsAll(context.transactionsCache(), Select(originalHashes, { 0, 2, 3, 5 }));
		test::AssertContainsAll(context.transactionsCache(), transactionData.Hashes);

		// - observer only gets called for entities that pass validation
		//   new: E[0] V0,O1,V1,O2; E[1] V2,O3,V3,O4; E[2] V4,O5,V5,O6
		//   old: E[0] V0,O1,V1,O2; E[1] V2; E[2] V2,O3,V3,O4; E[3] V4,O5,V5,O6; E[4] V6; E[5] V6,O7,V7,O8
		context.assertContexts(ConcatIds({ 0, 1, 2, 3, 4, 5 }, { 0, 1, 2, 2, 3, 4, 5, 6, 6, 7 }, 6));
		context.assertEntityInfos(
				ConcatContainers(transactionData.EntityInfos, originalTransactionData.EntityInfos),
				ConcatIds({ 0, 0, 1, 1, 2, 2 }, { 0, 0, 1, 2, 2, 3, 3, 4, 5, 5 }, 3),
				ConcatIds({ 0, 0, 1, 1, 2, 2 }, { 0, 0, 2, 2, 3, 3, 5, 5 }, 3),
				GetFailedIndexes(TResult, { { 3 + 1, TResult }, { 3 + 4, Modify(TResult) } }));
	}

	NON_SUCCESS_VALIDATION_TRAITS_BASED_TEST(OriginalTransactionsThatPartiallyFailValidationAreUndone) {
		// Arrange: initialize the UT cache with 6 transactions
		UpdaterTestContext context;

		// - note that the original transactions have timestamps following the new transaction timestamps below
		//   because the new transactions will be processed first
		auto originalTransactionData = CreateTransactionData(6, 3);
		const auto& originalHashes = originalTransactionData.Hashes;
		test::AddAll(context.transactionsCache(), originalTransactionData.UtInfos);

		// - prepare 3 new transactions
		auto transactionData = CreateTransactionData(3);

		// - set failures for 2 / 6 original entities
		context.setValidationResult(Modify(TResult), originalHashes[1], 2);
		context.setValidationResult(TResult, originalHashes[4], 2);

		// Sanity:
		EXPECT_EQ(6u, context.transactionsCache().view().size());

		// Act:
		context.updater().update({}, transactionData.UtInfos);

		// Assert:
		EXPECT_EQ(7u, context.transactionsCache().view().size());
		test::AssertContainsAll(context.transactionsCache(), Select(originalHashes, { 0, 2, 3, 5 }));
		test::AssertContainsAll(context.transactionsCache(), transactionData.Hashes);

		// - observer (commit) only gets called for entities that pass validation
		// - observer (rollback) gets called for entities that fail validation
		//   new: E[0] V0,O1,V1,O2; E[1] V2,O3,V3,O4; E[2] V4,O5,V5,O6
		//   old: E[0] V0,O1,V1,O2; E[1] V2,O3,V3;RO4 E[2] V4,O5,V5,O6; E[3] V6,O7,V7,O8; E[4] V8,O9,V9;RO10 E[5] V10,O11,V11,O12
		context.setPartialUndoFailureIndexes({ 6 + 3, 6 + 9 });
		context.assertContexts();
		context.assertEntityInfos(
				ConcatContainers(transactionData.EntityInfos, originalTransactionData.EntityInfos),
				GetFailedIndexes(TResult, { { 3 + 1, Modify(TResult) }, { 3 + 4, TResult } }));
	}

	TEST(TEST_CLASS, CommittedRevertedTransactionsAreAddedToCache) {
		// Arrange: initialize the UT cache with 3 transactions
		UpdaterTestContext context;
		auto originalTransactionData = CreateTransactionData(3);
		test::AddAll(context.transactionsCache(), originalTransactionData.UtInfos);

		// - prepare 6 new transactions
		auto transactionData = CreateTransactionData(6);
		const auto& hashes = transactionData.Hashes;

		// Sanity:
		EXPECT_EQ(3u, context.transactionsCache().view().size());

		// Act:
		context.updater().update({ &hashes[2], &hashes[4] }, transactionData.UtInfos);

		// Assert: the cache contains original and new transactions (and none were filtered out)
		EXPECT_EQ(9u, context.transactionsCache().view().size());
		test::AssertContainsAll(context.transactionsCache(), originalTransactionData.Hashes);
		test::AssertContainsAll(context.transactionsCache(), hashes);

		context.assertContexts();
		context.assertEntityInfos(ConcatContainers(transactionData.EntityInfos, originalTransactionData.EntityInfos));
	}

	TEST(TEST_CLASS, CommittedOriginalTransactionsAreNotAddedToCache) {
		// Arrange: initialize the UT cache with 6 transactions
		UpdaterTestContext context;
		auto originalTransactionData = CreateTransactionData(6);
		const auto& originalHashes = originalTransactionData.Hashes;
		test::AddAll(context.transactionsCache(), originalTransactionData.UtInfos);

		// - prepare 3 new transactions
		auto transactionData = CreateTransactionData(3);

		// Sanity:
		EXPECT_EQ(6u, context.transactionsCache().view().size());

		// Act:
		context.updater().update({ &originalHashes[2], &originalHashes[4] }, transactionData.UtInfos);

		// Assert: the cache contains original and new transactions (and the committed original ones were filtered out)
		EXPECT_EQ(7u, context.transactionsCache().view().size());
		test::AssertContainsAll(context.transactionsCache(), Select(originalHashes, { 0, 1, 3, 5 }));
		test::AssertContainsAll(context.transactionsCache(), transactionData.Hashes);

		context.assertContexts();

		// - validator and observer only get called for entities that pass the filter
		auto unconfirmedEntityInfos = ConcatContainers(
				transactionData.EntityInfos,
				Select(originalTransactionData.EntityInfos, { 0, 1, 3, 5 }));
		context.assertEntityInfos(unconfirmedEntityInfos);
	}

	// endregion
}}

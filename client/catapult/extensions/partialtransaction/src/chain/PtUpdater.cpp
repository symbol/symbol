#include "PtUpdater.h"
#include "PtValidator.h"
#include "partialtransaction/src/PtUtils.h"
#include "plugins/txes/aggregate/src/model/AggregateTransaction.h"
#include "catapult/cache/MemoryPtCache.h"
#include "catapult/crypto/Signer.h"
#include "catapult/thread/FutureUtils.h"
#include "catapult/thread/IoServiceThreadPool.h"
#include "catapult/utils/ArraySet.h"
#include "catapult/utils/HexFormatter.h"
#include "catapult/utils/MemoryUtils.h"
#include <boost/asio.hpp>

namespace catapult { namespace chain {

	namespace {
		using DetachedCosignatures = std::vector<model::DetachedCosignature>;

		std::shared_ptr<const model::AggregateTransaction> RemoveCosignatures(
				const std::shared_ptr<const model::AggregateTransaction>& pAggregateTransaction) {
			// if there are no cosignatures, no need to copy
			if (0 == pAggregateTransaction->CosignaturesCount())
				return pAggregateTransaction;

			// copy the transaction data without cosignatures
			uint32_t truncatedSize = sizeof(model::AggregateTransaction) + pAggregateTransaction->PayloadSize;
			auto pTransactionWithoutCosignatures = utils::MakeSharedWithSize<model::AggregateTransaction>(truncatedSize);
			memcpy(pTransactionWithoutCosignatures.get(), pAggregateTransaction.get(), truncatedSize);
			pTransactionWithoutCosignatures->Size = truncatedSize;
			return pTransactionWithoutCosignatures;
		}

		DetachedCosignatures ExtractCosignatures(
				const model::AggregateTransaction& aggregateTransaction,
				const Hash256& aggregateHash,
				const model::WeakCosignedTransactionInfo& transactionInfoFromCache) {
			utils::HashSet cosigners;
			DetachedCosignatures cosignatures;
			const auto* pCosignature = aggregateTransaction.CosignaturesPtr();
			for (auto i = 0u; i < aggregateTransaction.CosignaturesCount(); ++i) {
				const auto& cosigner = pCosignature->Signer;
				if (cosigners.emplace(cosigner).second && (!transactionInfoFromCache || !transactionInfoFromCache.hasCosigner(cosigner)))
					cosignatures.emplace_back(pCosignature->Signer, pCosignature->Signature, aggregateHash);

				++pCosignature;
			}

			return cosignatures;
		}
	}

	struct CheckEligibilityResult {
	public:
		constexpr CheckEligibilityResult(CosignatureUpdateResult updateResult, CosignersValidationResult validationResult)
				: m_updateResult(updateResult)
				, m_validationResult(validationResult)
		{}

	public:
		constexpr CosignatureUpdateResult updateResult() const {
			return m_updateResult;
		}

		constexpr bool isEligibile() const {
			return CosignersValidationResult::Missing == m_validationResult || CosignersValidationResult::Success == m_validationResult;
		}

		constexpr bool isPurgeRequired() const {
			return CosignersValidationResult::Failure == m_validationResult;
		}

	private:
		CosignatureUpdateResult m_updateResult;
		CosignersValidationResult m_validationResult;
	};

	class PtUpdater::Impl final : public std::enable_shared_from_this<PtUpdater::Impl> {
	public:
		Impl(
				cache::MemoryPtCacheProxy& transactionsCache,
				std::unique_ptr<const PtValidator>&& pValidator,
				const CompletedTransactionSink& completedTransactionSink,
				const FailedTransactionSink& failedTransactionSink,
				const std::shared_ptr<thread::IoServiceThreadPool>& pPool)
				: m_transactionsCache(transactionsCache)
				, m_pValidator(std::move(pValidator))
				, m_completedTransactionSink(completedTransactionSink)
				, m_failedTransactionSink(failedTransactionSink)
				, m_pPool(pPool)
		{}

	private:
		struct TransactionUpdateContext {
			std::shared_ptr<const model::AggregateTransaction> pAggregateTransaction;
			Hash256 AggregateHash;
			DetachedCosignatures Cosignatures;
			std::shared_ptr<const model::AddressSet> pExtractedAddresses;
		};

	public:
		thread::future<TransactionUpdateResult> update(const model::TransactionInfo& transactionInfo) {
			if (model::Entity_Type_Aggregate_Bonded != transactionInfo.pEntity->Type)
				CATAPULT_THROW_INVALID_ARGUMENT("PtUpdater only supports bonded aggregate transactions");

			const auto& aggregateHash = transactionInfo.EntityHash;
			DetachedCosignatures cosignatures;
			auto pAggregateTransaction = std::static_pointer_cast<const model::AggregateTransaction>(transactionInfo.pEntity);
			{
				auto view = m_transactionsCache.view();
				auto transactionInfoFromCache = view.find(aggregateHash);

				cosignatures = ExtractCosignatures(*pAggregateTransaction, aggregateHash, transactionInfoFromCache);

				if (transactionInfoFromCache)
					return update(cosignatures, TransactionUpdateResult::UpdateType::Existing);
			}

			auto pPromise = std::make_shared<thread::promise<TransactionUpdateResult>>(); // needs to be copyable to pass to post
			auto updateFuture = pPromise->get_future();

			TransactionUpdateContext updateContext;
			updateContext.pAggregateTransaction = pAggregateTransaction;
			updateContext.AggregateHash = aggregateHash;
			updateContext.Cosignatures = std::move(cosignatures);
			updateContext.pExtractedAddresses = transactionInfo.OptionalExtractedAddresses;
			m_pPool->service().post([pThis = shared_from_this(), updateContext, pPromise]() {
				pThis->updateImpl(updateContext).then([pPromise](auto&& resultFuture) {
					pPromise->set_value(resultFuture.get());
				});
			});

			return updateFuture;
		}

	private:
		thread::future<TransactionUpdateResult> updateImpl(const TransactionUpdateContext& updateContext) {
			const auto& aggregateHash = updateContext.AggregateHash;
			auto pAggregateTransactionWithoutCosignatures = RemoveCosignatures(updateContext.pAggregateTransaction);
			if (!isValid(*pAggregateTransactionWithoutCosignatures, aggregateHash))
				return thread::make_ready_future(TransactionUpdateResult{ TransactionUpdateResult::UpdateType::Invalid, 0 });

			// notice that the merkle component hash is not stored in the pt cache
			auto transactionInfo = model::DetachedTransactionInfo(pAggregateTransactionWithoutCosignatures, aggregateHash);
			transactionInfo.OptionalExtractedAddresses = updateContext.pExtractedAddresses;
			m_transactionsCache.modifier().add(transactionInfo);

			// if no cosignatures are present, check if the aggregate doesn't require any cosignatures (e.g. 1-of-1)
			if (updateContext.Cosignatures.empty())
				checkCompleteness(aggregateHash);

			return update(updateContext.Cosignatures, TransactionUpdateResult::UpdateType::New);
		}

	public:
		thread::future<CosignatureUpdateResult> update(const model::DetachedCosignature& cosignature) {
			auto pPromise = std::make_shared<thread::promise<CosignatureUpdateResult>>(); // needs to be copyable to pass to post
			auto updateFuture = pPromise->get_future();

			m_pPool->service().post([pThis = shared_from_this(), cosignature, pPromise]() {
				auto result = pThis->updateImpl(cosignature);
				pPromise->set_value(std::move(result));
			});

			return updateFuture;
		}

	private:
		CosignatureUpdateResult updateImpl(const model::DetachedCosignature& cosignature) {
			auto eligiblityResult = checkEligibility(cosignature);
			if (!eligiblityResult.isEligibile()) {
				if (eligiblityResult.isPurgeRequired())
					remove(cosignature.ParentHash);

				return eligiblityResult.updateResult();
			}

			if (!crypto::Verify(cosignature.Signer, cosignature.ParentHash, cosignature.Signature)) {
				CATAPULT_LOG(debug)
						<< "ignoring unverifiable cosignature (signer = " << utils::HexFormat(cosignature.Signer)
						<< ", parentHash = " << utils::HexFormat(cosignature.ParentHash) << ")";
				return CosignatureUpdateResult::Unverifiable;
			}

			return addCosignature(cosignature);
		}

		thread::future<TransactionUpdateResult> update(
				const DetachedCosignatures& cosignatures,
				TransactionUpdateResult::UpdateType updateType) {
			if (cosignatures.empty())
				return thread::make_ready_future(TransactionUpdateResult{ updateType, 0u });

			std::vector<thread::future<CosignatureUpdateResult>> futures;
			for (const auto& cosignature : cosignatures)
				futures.emplace_back(update(cosignature));

			return thread::when_all(std::move(futures)).then([updateType](auto&& resultsFuture) {
				auto results = resultsFuture.get();
				auto numCosignaturesAdded = std::count_if(results.begin(), results.end(), [](auto& resultFuture) {
					auto result = resultFuture.get();
					return CosignatureUpdateResult::Added_Incomplete == result || CosignatureUpdateResult::Added_Complete == result;
				});

				return TransactionUpdateResult{ updateType, static_cast<size_t>(numCosignaturesAdded) };
			});
		}

		CosignatureUpdateResult addCosignature(const model::DetachedCosignature& cosignature) {
			{
				auto modifier = m_transactionsCache.modifier();
				if (!modifier.add(cosignature.ParentHash, cosignature.Signer, cosignature.Signature))
					return CosignatureUpdateResult::Redundant;
			}

			return checkCompleteness(cosignature.ParentHash);
		}

	private:
		CosignatureUpdateResult checkCompleteness(const Hash256& aggregateHash) {
			std::vector<model::Cosignature> completedCosignatures;
			{
				auto view = m_transactionsCache.view();
				auto transactionInfoFromCache = view.find(aggregateHash);
				if (!transactionInfoFromCache)
					return CosignatureUpdateResult::Redundant;

				if (!isComplete(transactionInfoFromCache))
					return CosignatureUpdateResult::Added_Incomplete;

				completedCosignatures = transactionInfoFromCache.cosignatures();
			}

			// if complete, remove from the cache
			auto removedInfo = remove(aggregateHash);
			if (removedInfo)
				handleComplete({ removedInfo.pEntity.get(), &completedCosignatures });

			return CosignatureUpdateResult::Added_Complete;
		}

		bool isValid(const model::Transaction& transaction, const Hash256& aggregateHash) const {
			auto result = m_pValidator->validatePartial(model::WeakEntityInfoT<model::Transaction>(transaction, aggregateHash));
			if (result.Normalized)
				return true;

			m_failedTransactionSink(transaction, aggregateHash, result.Raw);
			return false;
		}

		CheckEligibilityResult checkEligibility(const model::DetachedCosignature& cosignature) const {
			auto view = m_transactionsCache.view();
			auto transactionInfoFromCache = view.find(cosignature.ParentHash);
			if (!transactionInfoFromCache)
				return { CosignatureUpdateResult::Ineligible, CosignersValidationResult::Ineligible };

			if (transactionInfoFromCache.hasCosigner(cosignature.Signer))
				return { CosignatureUpdateResult::Redundant, CosignersValidationResult::Ineligible };

			// if the result after including the new cosignature is ineligible, the new cosigner must be ineligible
			auto cosignatures = transactionInfoFromCache.cosignatures();
			cosignatures.push_back({ cosignature.Signer, cosignature.Signature });
			auto result = m_pValidator->validateCosigners({ &transactionInfoFromCache.transaction(), &cosignatures });
			if (CosignersValidationResult::Failure == result.Normalized)
				m_failedTransactionSink(transactionInfoFromCache.transaction(), cosignature.ParentHash, result.Raw);

			// notice that CosignersUpdateResult::Ineligible will be propagated conditionally on result.Normalized
			return { CosignatureUpdateResult::Ineligible, result.Normalized };
		}

		bool isComplete(const model::WeakCosignedTransactionInfo& transactionInfo) const {
			return CosignersValidationResult::Success == m_pValidator->validateCosigners(transactionInfo).Normalized;
		}

		void handleComplete(const model::WeakCosignedTransactionInfo& transactionInfo) {
			m_completedTransactionSink(partialtransaction::StitchAggregate(transactionInfo));
		}

		model::DetachedTransactionInfo remove(const Hash256& aggregateHash) {
			auto modifier = m_transactionsCache.modifier();
			return modifier.remove(aggregateHash);
		}

	private:
		cache::MemoryPtCacheProxy& m_transactionsCache;
		std::unique_ptr<const PtValidator> m_pValidator;
		CompletedTransactionSink m_completedTransactionSink;
		FailedTransactionSink m_failedTransactionSink;
		std::shared_ptr<thread::IoServiceThreadPool> m_pPool;
	};

	PtUpdater::PtUpdater(
			cache::MemoryPtCacheProxy& transactionsCache,
			std::unique_ptr<const PtValidator>&& pValidator,
			const CompletedTransactionSink& completedTransactionSink,
			const FailedTransactionSink& failedTransactionSink,
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool)
			: m_pImpl(std::make_shared<Impl>(
					transactionsCache,
					std::move(pValidator),
					completedTransactionSink,
					failedTransactionSink,
					pPool))
	{}

	PtUpdater::~PtUpdater() = default;

	thread::future<TransactionUpdateResult> PtUpdater::update(const model::TransactionInfo& transactionInfo) {
		return m_pImpl->update(transactionInfo);
	}

	thread::future<CosignatureUpdateResult> PtUpdater::update(const model::DetachedCosignature& cosignature) {
		return m_pImpl->update(cosignature);
	}
}}

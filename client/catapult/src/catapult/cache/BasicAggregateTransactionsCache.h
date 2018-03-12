#pragma once
#include "catapult/model/TransactionChangeTracker.h"
#include "catapult/utils/ExceptionLogging.h"
#include <memory>

namespace catapult { namespace cache {

	/// A basic aggregate transactions cache modifier that supports adding and removing of transaction infos.
	/// \note Subscribers are only notified of \em net changes.
	template<typename TCacheTraits, typename TChangeSubscriberTraits>
	class BasicAggregateTransactionsCacheModifier : public TCacheTraits::CacheModifierType {
	private:
		using ChangeSubscriberType = typename TCacheTraits::ChangeSubscriberType;
		using CacheModifierProxyType = typename TCacheTraits::CacheModifierProxyType;
		using TransactionInfoType = typename TChangeSubscriberTraits::TransactionInfoType;

	public:
		using TCacheTraits::CacheModifierType::add;

	public:
		/// Creates an aggregate transactions cache modifier around \a modifier and \a subscriber.
		BasicAggregateTransactionsCacheModifier(CacheModifierProxyType&& modifier, ChangeSubscriberType& subscriber)
				: m_modifier(std::move(modifier))
				, m_subscriber(subscriber)
		{}

		/// Destroys the modifier and notifies subscribers of changes.
		~BasicAggregateTransactionsCacheModifier() noexcept(false) {
			try {
				flush();
			} catch (...) {
				// this will crash the process, but the alternative of flushing explicitly is not much better because it will also crash
				// the process since the (non-recoverable) exception will bubble up to an unhandled exception handler
				CATAPULT_LOG(fatal) << "error notifying subscribers of transaction changes: " << EXCEPTION_DIAGNOSTIC_MESSAGE();
				utils::CatapultLogFlush();
				throw;
			}
		}

	public:
		bool add(const TransactionInfoType& transactionInfo) override {
			if (!m_modifier.add(transactionInfo))
				return false;

			m_transactionChangeTracker.add(TChangeSubscriberTraits::ToTransactionInfo(transactionInfo));
			return true;
		}

		TransactionInfoType remove(const Hash256& hash) override {
			auto transactionInfo = m_modifier.remove(hash);
			if (transactionInfo)
				remove(transactionInfo);

			return transactionInfo;
		}

	protected:
		CacheModifierProxyType& modifier() {
			return m_modifier;
		}

		ChangeSubscriberType& subscriber() {
			return m_subscriber;
		}

		void remove(const TransactionInfoType& transactionInfo) {
			m_transactionChangeTracker.remove(TChangeSubscriberTraits::ToTransactionInfo(transactionInfo));
		}

	private:
		void flush() {
			const auto& removedTransactionInfos = m_transactionChangeTracker.removedTransactionInfos();
			if (!removedTransactionInfos.empty())
				TChangeSubscriberTraits::NotifyRemoves(m_subscriber, m_transactionChangeTracker.removedTransactionInfos());

			const auto& addedTransactionInfos = m_transactionChangeTracker.addedTransactionInfos();
			if (!addedTransactionInfos.empty())
				TChangeSubscriberTraits::NotifyAdds(m_subscriber, addedTransactionInfos);

			m_subscriber.flush();
			m_transactionChangeTracker.reset();
		}

	private:
		CacheModifierProxyType m_modifier;
		ChangeSubscriberType& m_subscriber;
		model::TransactionChangeTracker m_transactionChangeTracker;
	};

	/// A basic aggregate transactions cache that delegates to a wrapped cache and raises notifications on a subscriber.
	template<typename TCacheTraits, typename TAggregateCacheModifier>
	class BasicAggregateTransactionsCache : public TCacheTraits::CacheType {
	private:
		using CacheType = typename TCacheTraits::CacheType;
		using ChangeSubscriberType = typename TCacheTraits::ChangeSubscriberType;
		using CacheModifierProxyType = typename TCacheTraits::CacheModifierProxyType;

	public:
		/// Creates an aggregate transactions cache that delegates to \a cache and publishes transaction changes to \a pChangeSubscriber.
		BasicAggregateTransactionsCache(CacheType& cache, std::unique_ptr<ChangeSubscriberType>&& pChangeSubscriber)
				: m_cache(cache)
				, m_pChangeSubscriber(std::move(pChangeSubscriber))
		{}

	public:
		CacheModifierProxyType modifier() override {
			return CacheModifierProxyType(std::make_unique<TAggregateCacheModifier>(m_cache.modifier(), *m_pChangeSubscriber));
		}

	private:
		CacheType& m_cache;
		std::unique_ptr<ChangeSubscriberType> m_pChangeSubscriber;
	};
}}

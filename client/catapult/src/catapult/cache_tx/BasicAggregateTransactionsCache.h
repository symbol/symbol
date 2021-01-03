/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#pragma once
#include "catapult/model/TransactionChangeTracker.h"
#include <memory>

namespace catapult { namespace cache {

	/// Basic aggregate transactions cache modifier that supports adding and removing of transaction infos.
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
		~BasicAggregateTransactionsCacheModifier() noexcept(false) override {
			flush();
		}

	public:
		size_t size() const override {
			return m_modifier.size();
		}

		utils::FileSize memorySize() const override {
			return modifier().memorySize();
		}

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
		/// Gets the modifier.
		CacheModifierProxyType& modifier() {
			return m_modifier;
		}

		/// Gets the (const) modifier.
		const CacheModifierProxyType& modifier() const {
			return m_modifier;
		}

		/// Gets the subscriber.
		ChangeSubscriberType& subscriber() {
			return m_subscriber;
		}

		/// Removes \a transactionInfo from the cache.
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

	/// Basic aggregate transactions cache that delegates to a wrapped cache and raises notifications on a subscriber.
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

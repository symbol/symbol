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

#pragma once
#include "AccountStateCacheDelta.h"
#include "AccountStateCacheView.h"
#include "catapult/cache/BasicCache.h"

namespace catapult { namespace cache {

	using AccountStateBasicCache = BasicCache<
		AccountStateCacheDescriptor,
		AccountStateCacheTypes::BaseSets,
		AccountStateCacheTypes::Options,
		const HighValueAccounts&>;

	/// Cache composed of stateful account information.
	class BasicAccountStateCache : public AccountStateBasicCache {
	public:
		/// Creates a cache around \a config and \a options.
		BasicAccountStateCache(const CacheConfiguration& config, const AccountStateCacheTypes::Options& options)
				: BasicAccountStateCache(config, options, std::make_unique<HighValueAccounts>())
		{}

	private:
		BasicAccountStateCache(
				const CacheConfiguration& config,
				const AccountStateCacheTypes::Options& options,
				std::unique_ptr<HighValueAccounts>&& pHighValueAccounts)
				: AccountStateBasicCache(config, AccountStateCacheTypes::Options(options), *pHighValueAccounts)
				, m_pHighValueAccounts(std::move(pHighValueAccounts))
		{}

	public:
		/// Initializes the cache with \a highValueAccounts.
		void init(HighValueAccounts&& highValueAccounts) {
			*m_pHighValueAccounts = std::move(highValueAccounts);
		}

		/// Commits all pending changes to the underlying storage.
		/// \note This hides AccountStateBasicCache::commit.
		void commit(CacheDeltaType& delta) {
			auto highValueAccounts = delta.detachHighValueAccounts();
			AccountStateBasicCache::commit(delta);
			*m_pHighValueAccounts = std::move(highValueAccounts);
		}

	private:
		// unique pointer to allow reference to be valid after moves of this cache
		std::unique_ptr<HighValueAccounts> m_pHighValueAccounts;
	};

	/// Synchronized cache composed of stateful account information.
	class AccountStateCache : public SynchronizedCacheWithInit<BasicAccountStateCache> {
	public:
		DEFINE_CACHE_CONSTANTS(AccountState)

	public:
		/// Creates a cache around \a config and \a options.
		AccountStateCache(const CacheConfiguration& config, const AccountStateCacheTypes::Options& options)
				: SynchronizedCacheWithInit<BasicAccountStateCache>(BasicAccountStateCache(config, options))
				, m_networkIdentifier(options.NetworkIdentifier)
				, m_importanceGrouping(options.ImportanceGrouping)
		{}

	public:
		/// Gets the network identifier.
		model::NetworkIdentifier networkIdentifier() const {
			return m_networkIdentifier;
		}

		/// Gets the network importance grouping.
		uint64_t importanceGrouping() const {
			return m_importanceGrouping;
		}

	private:
		model::NetworkIdentifier m_networkIdentifier;
		uint64_t m_importanceGrouping;
	};
}}

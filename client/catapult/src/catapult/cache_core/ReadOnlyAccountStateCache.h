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
#include "catapult/model/NetworkInfo.h"
#include "catapult/state/AccountState.h"

namespace catapult {
	namespace cache {
		class BasicAccountStateCacheDelta;
		class BasicAccountStateCacheView;
	}
}

namespace catapult { namespace cache {

	/// A read-only overlay on top of an account cache.
	class ReadOnlyAccountStateCache {
	public:
		/// Creates a read-only overlay on top of \a cache.
		explicit ReadOnlyAccountStateCache(const BasicAccountStateCacheView& cache)
				: m_pCache(&cache)
				, m_pCacheDelta(nullptr)
		{}

		/// Creates a read-only overlay on top of \a cache.
		explicit ReadOnlyAccountStateCache(const BasicAccountStateCacheDelta& cache)
				: m_pCache(nullptr)
				, m_pCacheDelta(&cache)
		{}

	public:
		/// Gets the network identifier.
		model::NetworkIdentifier networkIdentifier() const;

		/// Gets the network importance grouping.
		uint64_t importanceGrouping() const;

	public:
		/// Returns the number of elements in the cache.
		size_t size() const;

		/// Searches for the given \a address in the cache.
		/// Returns \c true if it is found or \c false otherwise.
		bool contains(const Address& address) const;

		/// Searches for the given \a publicKey in the cache.
		/// Returns \c true if it is found or \c false otherwise.
		bool contains(const Key& publicKey) const;

		/// Returns account state for an account identified by \a address.
		const state::AccountState& get(const Address& address) const;

		/// Returns account state for an account identified by \a publicKey.
		const state::AccountState& get(const Key& publicKey) const;

		/// Returns account state for an account identified by \a address or \c nullptr if the account was not found.
		const state::AccountState* tryGet(const Address& address) const;

		/// Returns account state for an account identified by \a publicKey or \c nullptr if the account was not found.
		const state::AccountState* tryGet(const Key& publicKey) const;

	private:
		const BasicAccountStateCacheView* m_pCache;
		const BasicAccountStateCacheDelta* m_pCacheDelta;
	};
}}

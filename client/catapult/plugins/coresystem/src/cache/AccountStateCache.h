#pragma once
#include "AccountStateCacheDelta.h"
#include "AccountStateCacheView.h"
#include "catapult/cache/SynchronizedCache.h"

namespace catapult { namespace cache {

	/// Cache composed of stateful account information.
	class BasicAccountStateCache : public utils::MoveOnly {
	public:
		using CacheViewType = AccountStateCacheView;
		using CacheDeltaType = AccountStateCacheDelta;
		using CacheReadOnlyType = account_state_cache_types::CacheReadOnlyType;

	public:
		/// Creates an account state cache for the network specified by \a networkIdentifier
		/// with the importance grouping (\a importanceGrouping).
		explicit BasicAccountStateCache(model::NetworkIdentifier networkIdentifier, uint64_t importanceGrouping)
				: m_networkIdentifier(networkIdentifier)
				, m_importanceGrouping(importanceGrouping)
		{}

	public:
		/// Returns a locked view based on this cache.
		CacheViewType createView() const {
			return CacheViewType(
					m_networkIdentifier,
					m_importanceGrouping,
					m_stateByAddress,
					m_keyToAddress);
		}

		/// Returns a locked cache delta based on this cache.
		CacheDeltaType createDelta() {
			return createDelta(m_stateByAddress.rebase(), m_keyToAddress.rebase());
		}

		/// Returns a lockable cache delta based on this cache but without the ability
		/// to commit any changes to the original cache.
		CacheDeltaType createDetachedDelta() const {
			return createDelta(m_stateByAddress.rebaseDetached(), m_keyToAddress.rebaseDetached());
		}

		/// Commits all pending changes to the underlying storage.
		void commit(const CacheDeltaType&) {
			m_stateByAddress.commit();
			m_keyToAddress.commit();
		}

	private:
		inline CacheDeltaType createDelta(
				const std::shared_ptr<account_state_cache_types::address_account_state_map::BaseSetDeltaType>& pStateByAddressDelta,
				const std::shared_ptr<account_state_cache_types::key_address_map::BaseSetDeltaType>& pKeyToAddressDelta) const {
			return CacheDeltaType(m_networkIdentifier, m_importanceGrouping, pStateByAddressDelta, pKeyToAddressDelta);
		}

	private:
		model::NetworkIdentifier m_networkIdentifier;
		uint64_t m_importanceGrouping;
		account_state_cache_types::address_account_state_map::BaseSetType m_stateByAddress;
		account_state_cache_types::key_address_map::BaseSetType m_keyToAddress;
	};

	/// Synchronized cache composed of stateful account information.
	class AccountStateCache : public SynchronizedCache<BasicAccountStateCache> {
	public:
		/// The unique cache identifier.
		static constexpr size_t Id = 0;

		/// The cache friendly name.
		static constexpr auto Name = "AccountStateCache";

	public:
		/// Creates an account state cache for the network specified by \a networkIdentifier
		/// with the importance grouping (\a importanceGrouping).
		explicit AccountStateCache(model::NetworkIdentifier networkIdentifier, uint64_t importanceGrouping)
				: SynchronizedCache<BasicAccountStateCache>(BasicAccountStateCache(networkIdentifier, importanceGrouping))
				, m_networkIdentifier(networkIdentifier)
				, m_importanceGrouping(importanceGrouping)
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

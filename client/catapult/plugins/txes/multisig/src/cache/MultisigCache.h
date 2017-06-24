#pragma once
#include "MultisigCacheDelta.h"
#include "MultisigCacheView.h"
#include "catapult/cache/SynchronizedCache.h"

namespace catapult { namespace cache {

	/// Cache composed of multisig information.
	class BasicMultisigCache : public utils::MoveOnly {
	public:
		using CacheViewType = MultisigCacheView;
		using CacheDeltaType = MultisigCacheDelta;
		using CacheReadOnlyType = multisig_cache_types::CacheReadOnlyType;

	private:
		// account entries map
		using PublicKeyBasedMultisigEntriesBaseSetDeltaPointerType =
				multisig_cache_types::account_multisig_entries_map::BaseSetDeltaPointerType;
		using PublicKeyBasedMultisigEntriesBaseSet = multisig_cache_types::account_multisig_entries_map::BaseSetType;

	public:
		/// Creates a multisig cache.
		BasicMultisigCache() = default;

	public:
		/// Returns a locked view based on this cache.
		CacheViewType createView() const {
			return CacheViewType(m_multisigEntries);
		}

		/// Returns a locked cache delta based on this cache.
		CacheDeltaType createDelta() {
			return createDelta(m_multisigEntries.rebase());
		}

		/// Returns a lockable cache delta based on this cache but without the ability
		/// to commit any changes to the original cache.
		CacheDeltaType createDetachedDelta() const {
			return createDelta(m_multisigEntries.rebaseDetached());
		}

		/// Commits all pending changes to the underlying storage.
		void commit(const CacheDeltaType&) {
			m_multisigEntries.commit();
		}

	private:
		inline CacheDeltaType createDelta(const PublicKeyBasedMultisigEntriesBaseSetDeltaPointerType& pMultisigEntries) const {
			return CacheDeltaType(pMultisigEntries);
		}

	private:
		PublicKeyBasedMultisigEntriesBaseSet m_multisigEntries;
	};

	/// Synchronized cache composed of multisig information.
	class MultisigCache : public SynchronizedCache<BasicMultisigCache> {
	public:
		/// The unique cache identifier.
		static constexpr size_t Id = 5;

		/// The cache friendly name.
		static constexpr auto Name = "MultisigCache";

	public:
		/// Creates a multisig cache.
		explicit MultisigCache() : SynchronizedCache<BasicMultisigCache>(BasicMultisigCache())
		{}
	};
}}

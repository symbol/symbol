#pragma once
#include "MultisigCacheTypes.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "catapult/deltaset/BaseSetDelta.h"
#include <vector>

namespace catapult { namespace cache {

	/// Basic delta on top of the multisig cache.
	class BasicMultisigCacheDelta : public utils::MoveOnly {
	public:
		using ReadOnlyView = multisig_cache_types::CacheReadOnlyType;

	private:
		using PublicKeyBasedMultisigEntriesBaseSetDeltaPointerType =
				multisig_cache_types::account_multisig_entries_map::BaseSetDeltaPointerType;

	public:
		/// Creates a delta based on the multisig entries map (\a pMultisigEntries).
		explicit BasicMultisigCacheDelta(const PublicKeyBasedMultisigEntriesBaseSetDeltaPointerType& pMultisigEntries)
				: m_pMultisigEntries(pMultisigEntries)
		{}

	public:
		/// Gets the number of multisigs in the cache.
		size_t size() const;

	public:
		/// Gets a value indicating whether or not the cache contains an entry for the account with \a key.
		bool contains(const Key& key) const;

		/// Gets a const multisig entry specified by its account \a key.
		const state::MultisigEntry& get(const Key& key) const;

		/// Gets a mutable multisig entry specified by its account \a key.
		state::MultisigEntry& get(const Key& key);

	public:
		/// Inserts the multisig \a entry into the cache.
		void insert(const state::MultisigEntry& entry);

		/// Removes the multisig entry specified by its account \a key from the cache.
		void remove(const Key& key);

	public:
		/// Gets all modified multisig entries.
		/// \note Both updated and new entries are included.
		std::vector<const state::MultisigEntry*> modifiedEntries() const;

		/// Gets the keys of all removed multisig entries.
		std::vector<Key> removedEntries() const;

	private:
		PublicKeyBasedMultisigEntriesBaseSetDeltaPointerType m_pMultisigEntries;
	};

	/// Delta on top of the multisig cache.
	class MultisigCacheDelta : public ReadOnlyViewSupplier<BasicMultisigCacheDelta> {
	public:
		/// Creates a delta around \a pMultisigEntries.
		explicit MultisigCacheDelta(const multisig_cache_types::account_multisig_entries_map::BaseSetDeltaPointerType& pMultisigEntries)
				: ReadOnlyViewSupplier(pMultisigEntries)
		{}
	};
}}

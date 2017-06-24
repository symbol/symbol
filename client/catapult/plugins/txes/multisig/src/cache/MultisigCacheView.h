#pragma once
#include "MultisigCacheTypes.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"

namespace catapult { namespace cache {

	/// Basic view on top of the multisig cache.
	class BasicMultisigCacheView : public utils::MoveOnly {
	public:
		using ReadOnlyView = multisig_cache_types::CacheReadOnlyType;

	private:
		using PublicKeyBasedMultisigEntriesBaseSet = multisig_cache_types::account_multisig_entries_map::BaseSetType;

	public:
		/// Creates a view around \a multisigEntries.
		explicit BasicMultisigCacheView(const PublicKeyBasedMultisigEntriesBaseSet& multisigEntries) : m_multisigEntries(multisigEntries)
		{}

	public:
		/// Gets the number of multisig entries in the cache.
		size_t size() const;

	public:
		/// Gets a value indicating whether or not the cache contains an entry for the account with \a key.
		bool contains(const Key& key) const;

		/// Gets a multisig entry specified by its account \a key.
		const state::MultisigEntry& get(const Key& key) const;

	public:
		/// Returns a const iterator to the first element of the underlying set.
		auto cbegin() const {
			return m_multisigEntries.cbegin();
		}

		/// Returns a const iterator to the element following the last element of the underlying set.
		auto cend() const {
			return m_multisigEntries.cend();
		}

	private:
		const PublicKeyBasedMultisigEntriesBaseSet& m_multisigEntries;
	};

	/// View on top of the multisig cache.
	class MultisigCacheView : public ReadOnlyViewSupplier<BasicMultisigCacheView> {
	public:
		/// Creates a view around \a multisigEntries.
		explicit MultisigCacheView(const multisig_cache_types::account_multisig_entries_map::BaseSetType& multisigEntries)
				: ReadOnlyViewSupplier(multisigEntries)
		{}
	};
}}

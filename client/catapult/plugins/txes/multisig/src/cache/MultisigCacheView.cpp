#include "MultisigCacheView.h"
#include "catapult/utils/HexFormatter.h"

namespace catapult { namespace cache {

	namespace {
		using PublicKeyBasedMultisigEntriesBaseSet = multisig_cache_types::account_multisig_entries_map::BaseSetType;

		auto& GetEntry(const PublicKeyBasedMultisigEntriesBaseSet& multisigEntries, const Key& key) {
			auto* pEntry = multisigEntries.find(key);
			if (!pEntry)
				CATAPULT_THROW_INVALID_ARGUMENT_1("account not found", utils::HexFormat(key));

			return *pEntry;
		}
	}

	size_t BasicMultisigCacheView::size() const {
		return m_multisigEntries.size();
	}

	bool BasicMultisigCacheView::contains(const Key& key) const {
		return m_multisigEntries.contains(key);
	}

	const state::MultisigEntry& BasicMultisigCacheView::get(const Key& key) const {
		return GetEntry(m_multisigEntries, key);
	}
}}

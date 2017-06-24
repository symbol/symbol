#include "MultisigCacheDelta.h"
#include "catapult/utils/Casting.h"
#include "catapult/utils/HexFormatter.h"

namespace catapult { namespace cache {

	namespace {
		template<typename TMultisigEntries>
		auto& GetEntry(TMultisigEntries& multisigEntries, const Key& key) {
			auto* pEntry = multisigEntries.find(key);
			if (!pEntry)
				CATAPULT_THROW_INVALID_ARGUMENT_1("account not found", utils::HexFormat(key));

			return *pEntry;
		}
	}

	size_t BasicMultisigCacheDelta::size() const {
		return m_pMultisigEntries->size();
	}

	bool BasicMultisigCacheDelta::contains(const Key& key) const {
		return m_pMultisigEntries->contains(key);
	}

	const state::MultisigEntry& BasicMultisigCacheDelta::get(const Key& key) const {
		return GetEntry(utils::as_const(*m_pMultisigEntries), key);
	}

	state::MultisigEntry& BasicMultisigCacheDelta::get(const Key& key) {
		return GetEntry(*m_pMultisigEntries, key);
	}

	void BasicMultisigCacheDelta::insert(const state::MultisigEntry& entry) {
		m_pMultisigEntries->insert(entry);
	}

	void BasicMultisigCacheDelta::remove(const Key& key) {
		m_pMultisigEntries->remove(key);
	}

	namespace {
		template<typename TDestination, typename TSource>
		void CollectAll(TDestination& dest, const TSource& source) {
			for (const auto& pair : source)
				dest.push_back(&pair.second);
		}
	}

	std::vector<const state::MultisigEntry*> BasicMultisigCacheDelta::modifiedEntries() const {
		std::vector<const state::MultisigEntry*> modifiedEntries;
		auto deltas = m_pMultisigEntries->deltas();
		CollectAll(modifiedEntries, deltas.Added);
		CollectAll(modifiedEntries, deltas.Copied);
		return modifiedEntries;
	}

	std::vector<Key> BasicMultisigCacheDelta::removedEntries() const {
		std::vector<Key> removedEntries;
		auto deltas = m_pMultisigEntries->deltas();
		for (const auto& pair : deltas.Removed)
			removedEntries.push_back(pair.first);

		return removedEntries;
	}
}}

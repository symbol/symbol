#include "UnlockedAccounts.h"
#include "catapult/utils/MacroBasedEnumIncludes.h"

namespace catapult { namespace harvesting {

#define DEFINE_ENUM UnlockedAccountsAddResult
#define ENUM_LIST UNLOCKED_ACCOUNTS_ADD_RESULT_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef DEFINE_ENUM

	namespace {
		auto CreateContainsPredicate(const Key& publicKey) {
			return [&publicKey](const auto& keyPair) {
				return keyPair.publicKey() == publicKey;
			};
		}
	}

	// region UnlockedAccountsView

	size_t UnlockedAccountsView::size() const {
		return m_keyPairs.size();
	}

	bool UnlockedAccountsView::contains(const Key& publicKey) const {
		return std::any_of(m_keyPairs.cbegin(), m_keyPairs.cend(), CreateContainsPredicate(publicKey));
	}

	// endregion

	// region UnlockedAccountsModifier

	UnlockedAccountsAddResult UnlockedAccountsModifier::add(crypto::KeyPair&& keyPair) {
		if (std::any_of(m_keyPairs.cbegin(), m_keyPairs.cend(), CreateContainsPredicate(keyPair.publicKey())))
			return UnlockedAccountsAddResult::Success;

		if (m_maxUnlockedAccounts == m_keyPairs.size())
			return UnlockedAccountsAddResult::Failure_Server_Limit;

		m_keyPairs.push_back(std::move(keyPair));
		return UnlockedAccountsAddResult::Success;
	}

	void UnlockedAccountsModifier::remove(const Key& publicKey) {
		auto iter = std::remove_if(m_keyPairs.begin(), m_keyPairs.end(), CreateContainsPredicate(publicKey));
		if (m_keyPairs.end() != iter)
			m_keyPairs.erase(iter);
	}

	void UnlockedAccountsModifier::removeIf(const KeyPredicate& predicate) {
		auto initialSize = m_keyPairs.size();
		auto keyPairPredicate = [predicate](const crypto::KeyPair& keyPair) { return predicate(keyPair.publicKey()); };
		m_keyPairs.erase(std::remove_if(m_keyPairs.begin(), m_keyPairs.end(), keyPairPredicate), m_keyPairs.end());
		if (m_keyPairs.size() != initialSize)
			CATAPULT_LOG(info) << "pruned " << (initialSize - m_keyPairs.size()) << " unlocked accounts";
	}

	// endregion

	// region UnlockedAccounts

	UnlockedAccountsView UnlockedAccounts::view() const {
		return UnlockedAccountsView(m_keyPairs, m_lock.acquireReader());
	}

	UnlockedAccountsModifier UnlockedAccounts::modifier() {
		return UnlockedAccountsModifier(m_maxUnlockedAccounts, m_keyPairs, m_lock.acquireReader());
	}

	// endregion
}}

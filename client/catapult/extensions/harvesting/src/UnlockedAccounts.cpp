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
		auto newKeyPairsEnd = std::remove_if(m_keyPairs.begin(), m_keyPairs.end(), [predicate](const auto& keyPair) {
			return predicate(keyPair.publicKey());
		});

		m_keyPairs.erase(newKeyPairsEnd, m_keyPairs.end());
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

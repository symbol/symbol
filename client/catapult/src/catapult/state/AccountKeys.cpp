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

#include "AccountKeys.h"

namespace catapult { namespace state {

	// region KeyAccessor

	template<typename TAccountPublicKey>
	AccountKeys::KeyAccessor<TAccountPublicKey>::KeyAccessor() = default;

	template<typename TAccountPublicKey>
	AccountKeys::KeyAccessor<TAccountPublicKey>::KeyAccessor(const KeyAccessor& keyAccessor) {
		*this = keyAccessor;
	}

	template<typename TAccountPublicKey>
	AccountKeys::KeyAccessor<TAccountPublicKey>::KeyAccessor(KeyAccessor&&) = default;

	template<typename TAccountPublicKey>
	AccountKeys::KeyAccessor<TAccountPublicKey>& AccountKeys::KeyAccessor<TAccountPublicKey>::operator=(const KeyAccessor& keyAccessor) {
		if (keyAccessor.m_pKey)
			m_pKey = std::make_shared<TAccountPublicKey>(*keyAccessor.m_pKey);
		else
			m_pKey.reset();

		return *this;
	}

	template<typename TAccountPublicKey>
	AccountKeys::KeyAccessor<TAccountPublicKey>& AccountKeys::KeyAccessor<TAccountPublicKey>::operator=(KeyAccessor&&) = default;

	template<typename TAccountPublicKey>
	AccountKeys::KeyAccessor<TAccountPublicKey>::operator bool() const {
		return m_pKey && TAccountPublicKey() != *m_pKey;
	}

	template<typename TAccountPublicKey>
	TAccountPublicKey AccountKeys::KeyAccessor<TAccountPublicKey>::get() const {
		return m_pKey ? *m_pKey : TAccountPublicKey();
	}

	template<typename TAccountPublicKey>
	void AccountKeys::KeyAccessor<TAccountPublicKey>::set(const TAccountPublicKey& key) {
		if (m_pKey)
			CATAPULT_THROW_INVALID_ARGUMENT("must call unset before resetting key with value");

		m_pKey = std::make_shared<TAccountPublicKey>(key);
	}

	template<typename TAccountPublicKey>
	void AccountKeys::KeyAccessor<TAccountPublicKey>::unset() {
		m_pKey.reset();
	}

	template class AccountKeys::KeyAccessor<Key>;
	template class AccountKeys::KeyAccessor<VotingKey>;

	// endregion

	// region AccountKeys

	AccountKeys::KeyType AccountKeys::mask() const {
		auto keyType = KeyType::Unset;
		keyType |= m_linkedPublicKey ? KeyType::Linked : KeyType::Unset;
		keyType |= m_vrfPublicKey ? KeyType::VRF : KeyType::Unset;
		keyType |= m_votingPublicKey ? KeyType::Voting : KeyType::Unset;
		keyType |= m_nodePublicKey ? KeyType::Node : KeyType::Unset;
		return keyType;
	}

	const AccountKeys::KeyAccessor<Key>& AccountKeys::linkedPublicKey() const {
		return m_linkedPublicKey;
	}

	AccountKeys::KeyAccessor<Key>& AccountKeys::linkedPublicKey() {
		return m_linkedPublicKey;
	}

	const AccountKeys::KeyAccessor<Key>& AccountKeys::vrfPublicKey() const {
		return m_vrfPublicKey;
	}

	AccountKeys::KeyAccessor<Key>& AccountKeys::vrfPublicKey() {
		return m_vrfPublicKey;
	}

	const AccountKeys::KeyAccessor<VotingKey>& AccountKeys::votingPublicKey() const {
		return m_votingPublicKey;
	}

	AccountKeys::KeyAccessor<VotingKey>& AccountKeys::votingPublicKey() {
		return m_votingPublicKey;
	}

	const AccountKeys::KeyAccessor<Key>& AccountKeys::nodePublicKey() const {
		return m_nodePublicKey;
	}

	AccountKeys::KeyAccessor<Key>& AccountKeys::nodePublicKey() {
		return m_nodePublicKey;
	}

	// endregion
}}

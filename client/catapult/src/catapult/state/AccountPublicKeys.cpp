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

#include "AccountPublicKeys.h"

namespace catapult { namespace state {

	// region PublicKeyAccessor

	template<typename TAccountPublicKey>
	AccountPublicKeys::PublicKeyAccessor<TAccountPublicKey>::PublicKeyAccessor() = default;

	template<typename TAccountPublicKey>
	AccountPublicKeys::PublicKeyAccessor<TAccountPublicKey>::PublicKeyAccessor(const PublicKeyAccessor& accessor) {
		*this = accessor;
	}

	template<typename TAccountPublicKey>
	AccountPublicKeys::PublicKeyAccessor<TAccountPublicKey>::PublicKeyAccessor(PublicKeyAccessor&&) = default;

	template<typename TAccountPublicKey>
	AccountPublicKeys::PublicKeyAccessor<TAccountPublicKey>& AccountPublicKeys::PublicKeyAccessor<TAccountPublicKey>::operator=(
			const PublicKeyAccessor& accessor) {
		if (accessor.m_pKey)
			m_pKey = std::make_shared<TAccountPublicKey>(*accessor.m_pKey);
		else
			m_pKey.reset();

		return *this;
	}

	template<typename TAccountPublicKey>
	AccountPublicKeys::PublicKeyAccessor<TAccountPublicKey>& AccountPublicKeys::PublicKeyAccessor<TAccountPublicKey>::operator=(
			PublicKeyAccessor&&) = default;

	template<typename TAccountPublicKey>
	AccountPublicKeys::PublicKeyAccessor<TAccountPublicKey>::operator bool() const {
		return m_pKey && TAccountPublicKey() != *m_pKey;
	}

	template<typename TAccountPublicKey>
	TAccountPublicKey AccountPublicKeys::PublicKeyAccessor<TAccountPublicKey>::get() const {
		return m_pKey ? *m_pKey : TAccountPublicKey();
	}

	template<typename TAccountPublicKey>
	void AccountPublicKeys::PublicKeyAccessor<TAccountPublicKey>::set(const TAccountPublicKey& key) {
		if (m_pKey)
			CATAPULT_THROW_INVALID_ARGUMENT("must call unset before resetting key with value");

		m_pKey = std::make_shared<TAccountPublicKey>(key);
	}

	template<typename TAccountPublicKey>
	void AccountPublicKeys::PublicKeyAccessor<TAccountPublicKey>::unset() {
		m_pKey.reset();
	}

	template class AccountPublicKeys::PublicKeyAccessor<Key>;
	template class AccountPublicKeys::PublicKeyAccessor<PinnedVotingKey>;

	// endregion

	// region AccountPublicKeys

	AccountPublicKeys::KeyType AccountPublicKeys::mask() const {
		auto keyType = KeyType::Unset;
		keyType |= m_linkedPublicKeyAccessor ? KeyType::Linked : KeyType::Unset;
		keyType |= m_nodePublicKeyAccessor ? KeyType::Node : KeyType::Unset;
		keyType |= m_vrfPublicKeyAccessor ? KeyType::VRF : KeyType::Unset;
		keyType |= m_votingPublicKeyAccessor ? KeyType::Voting : KeyType::Unset;
		return keyType;
	}

	const AccountPublicKeys::PublicKeyAccessor<Key>& AccountPublicKeys::linked() const {
		return m_linkedPublicKeyAccessor;
	}

	AccountPublicKeys::PublicKeyAccessor<Key>& AccountPublicKeys::linked() {
		return m_linkedPublicKeyAccessor;
	}

	const AccountPublicKeys::PublicKeyAccessor<Key>& AccountPublicKeys::node() const {
		return m_nodePublicKeyAccessor;
	}

	AccountPublicKeys::PublicKeyAccessor<Key>& AccountPublicKeys::node() {
		return m_nodePublicKeyAccessor;
	}

	const AccountPublicKeys::PublicKeyAccessor<Key>& AccountPublicKeys::vrf() const {
		return m_vrfPublicKeyAccessor;
	}

	AccountPublicKeys::PublicKeyAccessor<Key>& AccountPublicKeys::vrf() {
		return m_vrfPublicKeyAccessor;
	}

	const AccountPublicKeys::PublicKeyAccessor<PinnedVotingKey>& AccountPublicKeys::voting() const {
		return m_votingPublicKeyAccessor;
	}

	AccountPublicKeys::PublicKeyAccessor<PinnedVotingKey>& AccountPublicKeys::voting() {
		return m_votingPublicKeyAccessor;
	}

	// endregion
}}

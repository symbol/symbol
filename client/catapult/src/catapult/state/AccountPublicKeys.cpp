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

#define PUBLIC_KEY_ACCESSOR_T AccountPublicKeys::PublicKeyAccessor<TAccountPublicKey>
#define PUBLIC_KEYS_ACCESSOR_T AccountPublicKeys::PublicKeysAccessor<TPinnedAccountPublicKey>

	// region PublicKeyAccessor

	template<typename TAccountPublicKey>
	PUBLIC_KEY_ACCESSOR_T::PublicKeyAccessor() = default;

	template<typename TAccountPublicKey>
	PUBLIC_KEY_ACCESSOR_T::PublicKeyAccessor(const PublicKeyAccessor& accessor) {
		*this = accessor;
	}

	template<typename TAccountPublicKey>
	PUBLIC_KEY_ACCESSOR_T::PublicKeyAccessor(PublicKeyAccessor&&) = default;

	template<typename TAccountPublicKey>
	PUBLIC_KEY_ACCESSOR_T& PUBLIC_KEY_ACCESSOR_T::operator=(const PublicKeyAccessor& accessor) {
		if (accessor.m_pKey)
			m_pKey = std::make_shared<TAccountPublicKey>(*accessor.m_pKey);
		else
			m_pKey.reset();

		return *this;
	}

	template<typename TAccountPublicKey>
	PUBLIC_KEY_ACCESSOR_T& PUBLIC_KEY_ACCESSOR_T::operator=(PublicKeyAccessor&&) = default;

	template<typename TAccountPublicKey>
	PUBLIC_KEY_ACCESSOR_T::operator bool() const {
		return m_pKey && TAccountPublicKey() != *m_pKey;
	}

	template<typename TAccountPublicKey>
	TAccountPublicKey PUBLIC_KEY_ACCESSOR_T::get() const {
		return m_pKey ? *m_pKey : TAccountPublicKey();
	}

	template<typename TAccountPublicKey>
	void PUBLIC_KEY_ACCESSOR_T::set(const TAccountPublicKey& key) {
		if (m_pKey)
			CATAPULT_THROW_INVALID_ARGUMENT("must call unset before resetting key with value");

		m_pKey = std::make_shared<TAccountPublicKey>(key);
	}

	template<typename TAccountPublicKey>
	void PUBLIC_KEY_ACCESSOR_T::unset() {
		m_pKey.reset();
	}

	template class AccountPublicKeys::PublicKeyAccessor<Key>;

	// endregion

	// region PublicKeysAccessor

	template<typename TPinnedAccountPublicKey>
	PUBLIC_KEYS_ACCESSOR_T::PublicKeysAccessor() = default;

	template<typename TPinnedAccountPublicKey>
	PUBLIC_KEYS_ACCESSOR_T::PublicKeysAccessor(const PublicKeysAccessor& accessor) {
		*this = accessor;
	}

	template<typename TPinnedAccountPublicKey>
	PUBLIC_KEYS_ACCESSOR_T::PublicKeysAccessor(PublicKeysAccessor&&) = default;

	template<typename TPinnedAccountPublicKey>
	PUBLIC_KEYS_ACCESSOR_T& PUBLIC_KEYS_ACCESSOR_T::operator=(const PublicKeysAccessor& accessor) {
		if (accessor.m_pKeys)
			m_pKeys = std::make_shared<std::vector<TPinnedAccountPublicKey>>(*accessor.m_pKeys);
		else
			m_pKeys.reset();

		return *this;
	}

	template<typename TPinnedAccountPublicKey>
	PUBLIC_KEYS_ACCESSOR_T& PUBLIC_KEYS_ACCESSOR_T::operator=(PublicKeysAccessor&&) = default;

	template<typename TPinnedAccountPublicKey>
	size_t PUBLIC_KEYS_ACCESSOR_T::size() const {
		return m_pKeys ? m_pKeys->size() : 0;
	}

	template<typename TPinnedAccountPublicKey>
	FinalizationPoint PUBLIC_KEYS_ACCESSOR_T::upperBound() const {
		return m_pKeys ? m_pKeys->back().EndPoint : FinalizationPoint();
	}

	template<typename TPinnedAccountPublicKey>
	std::pair<size_t, bool> PUBLIC_KEYS_ACCESSOR_T::contains(FinalizationPoint point) const {
		if (!m_pKeys)
			return std::make_pair(std::numeric_limits<size_t>::max(), false);

		auto iter = std::find_if(m_pKeys->cbegin(), m_pKeys->cend(), [point](const auto& key) {
			return key.StartPoint <= point && point <= key.EndPoint;
		});
		return m_pKeys->cend() == iter
				? std::make_pair(std::numeric_limits<size_t>::max(), false)
				: std::make_pair(static_cast<size_t>(std::distance(m_pKeys->cbegin(), iter)), true);
	}

	template<typename TPinnedAccountPublicKey>
	bool PUBLIC_KEYS_ACCESSOR_T::containsExact(const TPinnedAccountPublicKey& key) const {
		return m_pKeys && m_pKeys->cend() != findExact(key);
	}

	template<typename TPinnedAccountPublicKey>
	const TPinnedAccountPublicKey& PUBLIC_KEYS_ACCESSOR_T::get(size_t index) const {
		return (*m_pKeys)[index];
	}

	template<typename TPinnedAccountPublicKey>
	std::vector<TPinnedAccountPublicKey> PUBLIC_KEYS_ACCESSOR_T::getAll() const {
		return m_pKeys ? *m_pKeys : std::vector<TPinnedAccountPublicKey>();
	}

	template<typename TPinnedAccountPublicKey>
	void PUBLIC_KEYS_ACCESSOR_T::add(const TPinnedAccountPublicKey& key) {
		if (upperBound() >= key.StartPoint)
			CATAPULT_THROW_INVALID_ARGUMENT("cannot add out of order public key");

		if (!m_pKeys)
			m_pKeys = std::make_shared<std::vector<TPinnedAccountPublicKey>>();

		m_pKeys->push_back(key);
	}

	template<typename TPinnedAccountPublicKey>
	bool PUBLIC_KEYS_ACCESSOR_T::remove(const TPinnedAccountPublicKey& key) {
		if (!m_pKeys)
			return false;

		auto iter = findExact(key);
		if (m_pKeys->cend() == iter)
			return false;

		m_pKeys->erase(iter);
		if (m_pKeys->empty())
			m_pKeys.reset();

		return true;
	}

	template<typename TPinnedAccountPublicKey>
	typename PUBLIC_KEYS_ACCESSOR_T::const_iterator PUBLIC_KEYS_ACCESSOR_T::findExact(const TPinnedAccountPublicKey& key) const {
		return std::find_if(m_pKeys->cbegin(), m_pKeys->cend(), [&key](const auto& existingKey) {
			return key.VotingKey == existingKey.VotingKey
					&& key.StartPoint == existingKey.StartPoint
					&& key.EndPoint == existingKey.EndPoint;
		});
	}

	template class AccountPublicKeys::PublicKeysAccessor<model::PinnedVotingKey>;

	// endregion

	// region AccountPublicKeys

	AccountPublicKeys::KeyType AccountPublicKeys::mask() const {
		auto keyType = KeyType::Unset;
		keyType |= m_linkedPublicKeyAccessor ? KeyType::Linked : KeyType::Unset;
		keyType |= m_nodePublicKeyAccessor ? KeyType::Node : KeyType::Unset;
		keyType |= m_vrfPublicKeyAccessor ? KeyType::VRF : KeyType::Unset;
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

	const AccountPublicKeys::PublicKeysAccessor<model::PinnedVotingKey>& AccountPublicKeys::voting() const {
		return m_votingPublicKeysAccessor;
	}

	AccountPublicKeys::PublicKeysAccessor<model::PinnedVotingKey>& AccountPublicKeys::voting() {
		return m_votingPublicKeysAccessor;
	}

	// endregion
}}

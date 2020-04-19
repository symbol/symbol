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
#include "catapult/utils/Casting.h"

namespace catapult { namespace state {

	namespace {
		template<typename TKeys>
		auto& Lookup(TKeys& keys, AccountKeyType keyType) {
			return keys[utils::to_underlying_type(keyType)];
		}

		bool IsValidKey(const std::shared_ptr<Key>& pKey) {
			return pKey && Key() != *pKey;
		}
	}

	AccountKeys::AccountKeys() = default;

	AccountKeys::AccountKeys(const AccountKeys& accountKeys) {
		*this = accountKeys;
	}

	AccountKeys::AccountKeys(AccountKeys&& accountKeys) = default;

	AccountKeys& AccountKeys::operator=(const AccountKeys& accountKeys) {
		for (auto i = 0u; i < std::tuple_size_v<KeysContainer>; ++i) {
			const auto& pSourceKey = accountKeys.m_keys[i];
			if (!pSourceKey)
				continue;

			m_keys[i] = std::make_shared<Key>(*pSourceKey);
		}

		return *this;
	}

	AccountKeys& AccountKeys::operator=(AccountKeys&& accountKeys) = default;

	size_t AccountKeys::size() const {
		return static_cast<size_t>(std::count_if(m_keys.cbegin(), m_keys.cend(), IsValidKey));
	}

	bool AccountKeys::contains(AccountKeyType keyType) const {
		return IsValidKey(Lookup(m_keys, keyType));
	}

	Key AccountKeys::get(AccountKeyType keyType) const {
		auto pKey = Lookup(m_keys, keyType);
		return pKey ? *pKey : Key();
	}

	void AccountKeys::set(AccountKeyType keyType, const Key& key) {
		if (contains(keyType))
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot set key multiple times", static_cast<uint16_t>(keyType));

		Lookup(m_keys, keyType) = std::make_shared<Key>(key);
	}

	void AccountKeys::unset(AccountKeyType keyType) {
		Lookup(m_keys, keyType) = std::shared_ptr<Key>();
	}
}}

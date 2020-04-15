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

#pragma once
#include "catapult/types.h"
#include <array>
#include <memory>

namespace catapult { namespace state {

	/// Types of account keys.
	enum class AccountKeyType : uint8_t {
		/// Linked account public key.
		/// \note This can be either a remote or main account public key depending on context.
		Linked,

		/// VRF public key.
		VRF,

		/// Voting public key.
		Voting,

		/// Number of account key types.
		Count
	};

	/// Container holding account key information.
	class AccountKeys {
	public:
		/// Creates empty account keys.
		AccountKeys();

		/// Copy constructor that makes a deep copy of \a accountKeys.
		AccountKeys(const AccountKeys& accountKeys);

		/// Move constructor that move constructs an account keys from \a accountKeys.
		AccountKeys(AccountKeys&& accountKeys);

	public:
		/// Assignment operator that makes a deep copy of \a accountKeys.
		AccountKeys& operator=(const AccountKeys& accountKeys);

		/// Move assignment operator that assigns \a accountKeys.
		AccountKeys& operator=(AccountKeys&& accountKeys);

	public:
		/// Gets the number of keys.
		size_t size() const;

		/// Returns \c true if a key of the specified type (\a keyType) is set.
		bool contains(AccountKeyType keyType) const;

		/// Gets the key associated with \a keyType or a zero key if no such key exists.
		Key get(AccountKeyType keyType) const;

	public:
		/// Sets an account \a key with \a keyType.
		void set(AccountKeyType keyType, const Key& key);

		/// Unsets the account key with \a keyType.
		void unset(AccountKeyType keyType);

	private:
		using KeysContainer = std::array<std::shared_ptr<Key>, static_cast<size_t>(AccountKeyType::Count)>;
		KeysContainer m_keys;
	};
}}

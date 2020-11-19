/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "catapult/crypto/KeyPair.h"
#include <vector>

namespace catapult { namespace test {

	/// Container of accounts.
	/// \note First account is always nemesis recipient account.
	class Accounts {
	public:
		/// Creates a container with \a numAccounts accounts.
		explicit Accounts(size_t numAccounts);

	public:
		/// Gets the address for the \a id account.
		Address getAddress(size_t id) const;

		/// Gets the key pair for the \a id account.
		const crypto::KeyPair& getKeyPair(size_t id) const;

	private:
		std::vector<crypto::KeyPair> m_keyPairs;
	};
}}

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

#include "Accounts.h"
#include "catapult/model/Address.h"
#include "catapult/exceptions.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/test/nodeps/TestNetworkConstants.h"

namespace catapult { namespace test {

	Accounts::Accounts(size_t numAccounts) {
		if (0 == numAccounts)
			CATAPULT_THROW_INVALID_ARGUMENT("must create at least one account");

		m_keyPairs.push_back(crypto::KeyPair::FromString(Test_Network_Private_Keys[0]));
		for (auto i = 0u; i < numAccounts - 1; ++i)
			m_keyPairs.push_back(GenerateKeyPair());
	}

	Address Accounts::getAddress(size_t id) const {
		return model::PublicKeyToAddress(getKeyPair(id).publicKey(), model::NetworkIdentifier::Private_Test);
	}

	const crypto::KeyPair& Accounts::getKeyPair(size_t id) const {
		if (id >= m_keyPairs.size())
			CATAPULT_THROW_INVALID_ARGUMENT_1("invalid account id", id);

		return m_keyPairs[id];
	}
}}


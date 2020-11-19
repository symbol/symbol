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
#include "catapult/utils/ArraySet.h"

namespace catapult { namespace test {

	/// Generates a random private key.
	crypto::PrivateKey GenerateRandomPrivateKey();

	/// Generates a random key pair.
	crypto::KeyPair GenerateKeyPair();

	/// Copies a given \a keyPair.
	crypto::KeyPair CopyKeyPair(const crypto::KeyPair& keyPair);

	/// Extracts the public keys of \a keyPairs into a key set.
	utils::KeySet ToKeySet(const std::vector<crypto::KeyPair>& keyPairs);
}}

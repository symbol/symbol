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
#include "catapult/crypto/KeyPair.h"
#include "catapult/crypto/PrivateKey.h"

namespace catapult { namespace test {

	/// Generates a random private key.
	crypto::PrivateKey GenerateRandomPrivateKey();

	/// Generates a random key pair.
	crypto::KeyPair GenerateKeyPair();

	/// Creates key pair around private key \a buffer.
	template<typename TContainer>
	crypto::KeyPair KeyPairFromPrivateKeyBuffer(const TContainer& buffer) {
		return crypto::KeyPair::FromPrivate(crypto::PrivateKey::Generate([iter = buffer.begin()]() mutable { return *iter++; }));
	}
}}

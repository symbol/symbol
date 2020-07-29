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
#include "catapult/crypto/AesDecrypt.h"

namespace catapult { namespace test {

	/// Encrypts \a input into \a output using \a iv and \a encryptionKey.
	void AesGcmEncrypt(
			const crypto::SharedKey& encryptionKey,
			const crypto::AesGcm256::IV& iv,
			const RawBuffer& input,
			std::vector<uint8_t>& output);

	/// Encrypts \a clearText with shared key derived from generated ephemeral key and \a recipientPublicKey.
	std::vector<uint8_t> GenerateEphemeralAndEncrypt(const RawBuffer& clearText, const Key& recipientPublicKey);
}}

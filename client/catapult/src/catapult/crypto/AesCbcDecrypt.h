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
#include "SharedKey.h"

namespace catapult { namespace crypto {

	struct AesInitializationVector_tag { static constexpr size_t Size = 16; };
	using AesInitializationVector = utils::ByteArray<AesInitializationVector_tag>;

	/// Decrypts \a input to \a output using AES with \a key in CBC mode.
	bool TryAesCbcDecrypt(const SharedKey& key, const RawBuffer& input, std::vector<uint8_t>& output);

	/// Extracts ephemeral public key from \a encryptedWithKey and decrypts rest to \a decrypted using \a keyPair.
	bool TryDecryptEd25199BlockCipher(const RawBuffer& encryptedWithKey, const KeyPair& keyPair, std::vector<uint8_t>& decrypted);
}}

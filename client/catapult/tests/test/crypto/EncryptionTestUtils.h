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
#include "catapult/crypto/AesCbcDecrypt.h"

namespace catapult { namespace test {

	/// Pads \a buffer using pkcs#7 padding using aes block size.
	void AesPkcs7PaddingScheme(std::vector<uint8_t>& buffer);

	/// Encrypts \a input with applied padding (\a applyPaddingScheme) into \a output using \a initializationVector and \a encryptionKey.
	void AesCbcEncrypt(
			const crypto::SharedKey& encryptionKey,
			const crypto::AesInitializationVector& initializationVector,
			const RawBuffer& input,
			std::vector<uint8_t>& output,
			const consumer<std::vector<uint8_t>&>& applyPaddingScheme = AesPkcs7PaddingScheme);

	/// Encrypts \a clearText with shared key derived from generated ephemeral key and \a recipientPublicKey.
	std::vector<uint8_t> GenerateEphemeralAndEncrypt(const RawBuffer& clearText, const Key& recipientPublicKey);
}}

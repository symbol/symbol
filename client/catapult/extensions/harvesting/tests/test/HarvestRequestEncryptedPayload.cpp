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

#include "HarvestRequestEncryptedPayload.h"
#include "catapult/io/BufferedFileStream.h"
#include "tests/test/crypto/EncryptionTestUtils.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/test/nodeps/Random.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	harvesting::HarvestRequestIdentifier GetRequestIdentifier(const HarvestRequestEncryptedPayload& encryptedPayload) {
		harvesting::HarvestRequestIdentifier requestIdentifier;
		std::memcpy(requestIdentifier.data(), encryptedPayload.Data.data(), requestIdentifier.size());
		return requestIdentifier;
	}

	std::ostream& operator<<(std::ostream& out, const HarvestRequestEncryptedPayload& encryptedPayload) {
		out
				<< "identifier:" << GetRequestIdentifier(encryptedPayload)
				<< ", data:" << utils::HexFormat(encryptedPayload.Data) << std::endl;
		return out;
	}

	HarvestRequestEncryptedPayload PrepareHarvestRequestEncryptedPayload(const Key& recipientPublicKey, const RawBuffer& clearTextBuffer) {
		return PrepareHarvestRequestEncryptedPayload(GenerateKeyPair(), recipientPublicKey, clearTextBuffer);
	}

	HarvestRequestEncryptedPayload PrepareHarvestRequestEncryptedPayload(
			const crypto::KeyPair& ephemeralKeyPair,
			const Key& recipientPublicKey,
			const RawBuffer& clearTextBuffer) {
		HarvestRequestEncryptedPayload encryptedPayload;
		auto sharedKey = crypto::DeriveSharedKey(ephemeralKeyPair, recipientPublicKey);
		auto iv = GenerateRandomByteArray<crypto::AesGcm256::IV>();

		std::vector<uint8_t> encrypted;
		AesGcmEncrypt(sharedKey, iv, clearTextBuffer, encrypted);

		std::memcpy(encryptedPayload.Data.data(), ephemeralKeyPair.publicKey().data(), Key::Size);
		std::memcpy(encryptedPayload.Data.data() + Key::Size, encrypted.data(), encrypted.size());
		return encryptedPayload;
	}

	std::vector<uint8_t> CopyHarvestRequestEncryptedPayloadToBuffer(const HarvestRequestEncryptedPayload& encryptedPayload) {
		std::vector<uint8_t> buffer(sizeof(HarvestRequestEncryptedPayload));
		std::memcpy(buffer.data(), &encryptedPayload, sizeof(HarvestRequestEncryptedPayload));
		return buffer;
	}

	void AssertHarvesterFileContents(const std::string& filename, const HarvestRequestEncryptedPayloads& expectedEncryptedPayloads) {
		// Assert:
		io::RawFile file(filename, io::OpenMode::Read_Only);
		EXPECT_EQ(expectedEncryptedPayloads.size() * sizeof(HarvestRequestEncryptedPayload), file.size());

		io::BufferedInputFileStream inputStream(std::move(file));

		// - read encrypted payloads directly from file
		HarvestRequestEncryptedPayloads actualEncryptedPayloads;
		while (!inputStream.eof()) {
			HarvestRequestEncryptedPayload encryptedPayload;
			inputStream.read({ reinterpret_cast<uint8_t*>(&encryptedPayload), sizeof(HarvestRequestEncryptedPayload) });
			actualEncryptedPayloads.emplace(encryptedPayload);
		}

		// - compare against expected encrypted payloads
		EXPECT_EQ(expectedEncryptedPayloads, actualEncryptedPayloads);
	}

	std::vector<harvesting::BlockGeneratorAccountDescriptor> GenerateRandomAccountDescriptors(size_t numDescriptors) {
		std::vector<harvesting::BlockGeneratorAccountDescriptor> descriptors;
		for (auto i = 0u; i < numDescriptors; ++i)
			descriptors.emplace_back(GenerateKeyPair(), GenerateKeyPair());

		return descriptors;
	}

	std::vector<uint8_t> ToClearTextBuffer(const harvesting::BlockGeneratorAccountDescriptor& descriptor) {
		std::vector<uint8_t> clearText(2 * Key::Size);
		std::memcpy(&clearText[0], descriptor.signingKeyPair().privateKey().data(), Key::Size);
		std::memcpy(&clearText[Key::Size], descriptor.vrfKeyPair().privateKey().data(), Key::Size);
		return clearText;
	}
}}

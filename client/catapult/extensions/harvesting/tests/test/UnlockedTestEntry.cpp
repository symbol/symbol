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

#include "UnlockedTestEntry.h"
#include "catapult/io/BufferedFileStream.h"
#include "tests/test/crypto/EncryptionTestUtils.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/test/nodeps/Random.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	namespace {
		void AesPkcs7MalformedPaddingScheme(std::vector<uint8_t>& buffer) {
			// apply normal padding first
			AesPkcs7PaddingScheme(buffer);

			auto paddingSize = buffer.back();

			// malform all pad bytes except last one
			for (auto i = 0u; i < static_cast<uint8_t>(paddingSize - 1); ++i)
				buffer[buffer.size() - paddingSize + i] ^= 0xFF;
		}
	}

	harvesting::UnlockedEntryMessageIdentifier GetMessageIdentifier(const UnlockedTestEntry& entry) {
		harvesting::UnlockedEntryMessageIdentifier messageIdentifier;
		std::memcpy(messageIdentifier.data(), entry.Payload.data(), messageIdentifier.size());
		return messageIdentifier;
	}

	std::ostream& operator<<(std::ostream& out, const UnlockedTestEntry& entry) {
		out << "identifier:" << GetMessageIdentifier(entry) << ", " << "payload:" << utils::HexFormat(entry.Payload) << std::endl;
		return out;
	}

	UnlockedTestEntry PrepareUnlockedTestEntry(
			const Key& recipientPublicKey,
			const RawBuffer& entryBuffer,
			EncryptionMutationFlag encryptionMutationFlag) {
		return PrepareUnlockedTestEntry(test::GenerateKeyPair(), recipientPublicKey, entryBuffer, encryptionMutationFlag);
	}

	UnlockedTestEntry PrepareUnlockedTestEntry(
			const crypto::KeyPair& ephemeralKeyPair,
			const Key& recipientPublicKey,
			const RawBuffer& entryBuffer,
			EncryptionMutationFlag encryptionMutationFlag) {
		UnlockedTestEntry entry;
		auto sharedKey = crypto::DeriveSharedKey(ephemeralKeyPair, recipientPublicKey);
		auto initializationVector = GenerateRandomByteArray<crypto::AesInitializationVector>();

		std::vector<uint8_t> encrypted;
		const auto& paddingScheme =
				EncryptionMutationFlag::None == encryptionMutationFlag
				? AesPkcs7PaddingScheme
				: AesPkcs7MalformedPaddingScheme;
		AesCbcEncrypt(sharedKey, initializationVector, entryBuffer, encrypted, paddingScheme);

		std::memcpy(entry.Payload.data(), ephemeralKeyPair.publicKey().data(), Key::Size);
		std::memcpy(entry.Payload.data() + Key::Size, encrypted.data(), encrypted.size());
		return entry;
	}

	std::vector<uint8_t> ConvertUnlockedTestEntryToBuffer(const UnlockedTestEntry& entry) {
		std::vector<uint8_t> buffer(sizeof(UnlockedTestEntry));
		std::memcpy(buffer.data(), &entry, sizeof(UnlockedTestEntry));
		return buffer;
	}

	void AssertUnlockedEntriesFileContents(const std::string& filename, const UnlockedTestEntries& expectedEntries) {
		// Assert:
		io::RawFile file(filename, io::OpenMode::Read_Only);
		EXPECT_EQ(expectedEntries.size() * sizeof(UnlockedTestEntry), file.size());

		io::BufferedInputFileStream inputStream(std::move(file));

		// - read entries directly from file
		UnlockedTestEntries actualEntries;
		while (!inputStream.eof()) {
			UnlockedTestEntry entry;
			inputStream.read({ reinterpret_cast<uint8_t*>(&entry), sizeof(UnlockedTestEntry) });
			actualEntries.emplace(entry);
		}

		// - compare against expected entries
		EXPECT_EQ(expectedEntries, actualEntries);
	}
}}

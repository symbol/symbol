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
#include "harvesting/src/UnlockedFileQueueConsumer.h"
#include <set>
#include <vector>

namespace catapult { namespace crypto { class KeyPair; } }

namespace catapult { namespace test {

#pragma pack(push, 1)

	/// Harvest request encrypted payload.
	struct HarvestRequestEncryptedPayload {
	public:
		/// Encrypted payload size.
		static constexpr size_t Size = 32u + 16 + 12 + 32 + 32;

	public:
		/// Encrypted data memory buffer.
		/// \note This decrypts into BlockGeneratorAccountDescriptor.
		std::array<uint8_t, Size> Data;

	public:
		/// Returns \c true if this payload is equal to \a rhs.
		bool operator==(const HarvestRequestEncryptedPayload& rhs) const {
			return Data == rhs.Data;
		}
	};

#pragma pack(pop)

	/// Gets a unique identifier for \a encryptedPayload.
	harvesting::HarvestRequestIdentifier GetRequestIdentifier(const HarvestRequestEncryptedPayload& encryptedPayload);

	/// Insertion operator for outputting \a encryptedPayload to \a out.
	std::ostream& operator<<(std::ostream& out, const HarvestRequestEncryptedPayload& encryptedPayload);

	/// Creates an encrypted payload around \a clearTextBuffer using \a recipientPublicKey.
	HarvestRequestEncryptedPayload PrepareHarvestRequestEncryptedPayload(const Key& recipientPublicKey, const RawBuffer& clearTextBuffer);

	/// Creates an encrypted payload around \a clearTextBuffer using \a ephemeralKeyPair and \a recipientPublicKey.
	HarvestRequestEncryptedPayload PrepareHarvestRequestEncryptedPayload(
			const crypto::KeyPair& ephemeralKeyPair,
			const Key& recipientPublicKey,
			const RawBuffer& clearTextBuffer);

	/// Copes \a encryptedPayload to a new buffer.
	std::vector<uint8_t> CopyHarvestRequestEncryptedPayloadToBuffer(const HarvestRequestEncryptedPayload& encryptedPayload);

	/// Harvest request encrypted payload comparator.
	struct HarvestRequestEncryptedPayloadComparator {
		bool operator()(const HarvestRequestEncryptedPayload& lhs, const HarvestRequestEncryptedPayload& rhs) const {
			return GetRequestIdentifier(lhs) < GetRequestIdentifier(rhs);
		}
	};

	/// Harvest request encrypted payloads.
	using HarvestRequestEncryptedPayloads = std::set<HarvestRequestEncryptedPayload, HarvestRequestEncryptedPayloadComparator>;

	/// Asserts that contents of file (\a filename) matches \a expectedEncryptedPayloads.
	void AssertHarvesterFileContents(const std::string& filename, const HarvestRequestEncryptedPayloads& expectedEncryptedPayloads);

	/// Generates \a numDescriptors random block generator account descriptors.
	std::vector<harvesting::BlockGeneratorAccountDescriptor> GenerateRandomAccountDescriptors(size_t numDescriptors);

	/// Serializes \a descriptor into a clear text buffer.
	std::vector<uint8_t> ToClearTextBuffer(const harvesting::BlockGeneratorAccountDescriptor& descriptor);
}}

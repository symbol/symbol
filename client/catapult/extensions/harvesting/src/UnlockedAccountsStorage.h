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
#include "HarvestRequest.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/functions.h"
#include "catapult/types.h"
#include <map>

namespace catapult { namespace harvesting { class BlockGeneratorAccountDescriptor; } }

namespace catapult { namespace harvesting {

	/// Unlocked accounts storage.
	class UnlockedAccountsStorage {
	private:
		using IdentityToEncryptedPayloadMap = std::map<HarvestRequestIdentifier, std::vector<uint8_t>>;

		// "request" refers to pair of request identifier and encrypted payload
		using IdentityRequestPair = IdentityToEncryptedPayloadMap::value_type;
		using RequestToHarvesterMap = std::map<IdentityRequestPair, Key>;

	public:
		/// Creates unlocked accounts storage around \a filename.
		explicit UnlockedAccountsStorage(const std::string& filename);

	public:
		/// Returns \c true if this storage contains a request identified by \a requestIdentifier.
		bool contains(const HarvestRequestIdentifier& requestIdentifier);

	public:
		/// Adds harvest request identified by \a requestIdentifier with encrypted payload (\a encryptedPayload)
		/// and associated \a harvesterPublicKey.
		void add(const HarvestRequestIdentifier& requestIdentifier, const RawBuffer& encryptedPayload, const Key& harvesterPublicKey);

		/// Removes harvest request identified by \a requestIdentifier.
		void remove(const HarvestRequestIdentifier& requestIdentifier);

		/// Saves harvest requests that pass \a filter.
		void save(const predicate<const Key&>& filter) const;

		/// Loads harvest requests using \a encryptionKeyPair and forwards to \a processDescriptor.
		void load(const crypto::KeyPair& encryptionKeyPair, const consumer<BlockGeneratorAccountDescriptor&&>& processDescriptor);

	private:
		void addRequest(
				const HarvestRequestIdentifier& requestIdentifier,
				const std::vector<uint8_t>& encryptedPayload,
				const Key& harvesterPublicKey);

		bool tryRemoveRequest(const HarvestRequestIdentifier& requestIdentifier);

	private:
		std::string m_filename;
		IdentityToEncryptedPayloadMap m_identityToEncryptedPayloadMap;
		RequestToHarvesterMap m_requestToHarvesterMap;
	};
}}

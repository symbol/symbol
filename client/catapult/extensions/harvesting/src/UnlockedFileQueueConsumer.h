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
#include "UnlockedEntryMessage.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/functions.h"
#include <string>

namespace catapult {
	namespace config { class CatapultDirectory; }
	namespace crypto { class KeyPair; }
}

namespace catapult { namespace harvesting {

	/// Decrypts \a encryptedWithKey using \a bootKeyPair.
	std::pair<crypto::PrivateKey, bool> TryDecryptUnlockedEntry(const RawBuffer& encryptedWithKey, const crypto::KeyPair& bootKeyPair);

	/// Reads encrypted unlocked entry messages from \a directory, validates using \a bootKeyPair and forwards to \a processEntryKeyPair.
	void UnlockedFileQueueConsumer(
			const config::CatapultDirectory& directory,
			const crypto::KeyPair& bootKeyPair,
			const consumer<const UnlockedEntryMessage&, crypto::KeyPair&&>& processEntryKeyPair);
}}

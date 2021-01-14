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
#include "UnlockedAccountsStorage.h"
#include "catapult/config/CatapultDataDirectory.h"

namespace catapult {
	namespace cache { class CatapultCache; }
	namespace harvesting { class UnlockedAccounts; }
}

namespace catapult { namespace harvesting {

	/// Unlocked accounts updater.
	class UnlockedAccountsUpdater {
	public:
		/// Creates unlocked accounts updater around \a cache, \a unlockedAccounts, node owner
		/// (identified by \a signingPublicKey and \a encryptionKeyPair) and \a dataDirectory.
		UnlockedAccountsUpdater(
				const cache::CatapultCache& cache,
				UnlockedAccounts& unlockedAccounts,
				const Key& signingPublicKey,
				const crypto::KeyPair& encryptionKeyPair,
				const config::CatapultDataDirectory& dataDirectory);

	public:
		/// Loads storage.
		void load();

		/// Updates unlocked accounts.
		void update();

	private:
		const cache::CatapultCache& m_cache;
		UnlockedAccounts& m_unlockedAccounts;
		Key m_signingPublicKey;
		const crypto::KeyPair& m_encryptionKeyPair;
		config::CatapultDataDirectory m_dataDirectory;
		std::string m_harvestersFilename;
		UnlockedAccountsStorage m_unlockedAccountsStorage;
	};
}}

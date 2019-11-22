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
#include "catapult/functions.h"
#include "catapult/types.h"
#include <map>

namespace catapult { namespace crypto { class KeyPair; } }

namespace catapult { namespace harvesting {

	/// Unlocked accounts storage.
	class UnlockedAccountsStorage {
	private:
		using AnnouncerToEntryMap = std::map<Key, std::vector<uint8_t>>;
		using AccountEntryPair = AnnouncerToEntryMap::value_type;
		using EntryToHarvesterMap = std::map<AccountEntryPair, Key>;

	public:
		/// Creates unlocked accounts storage around \a filename.
		explicit UnlockedAccountsStorage(const std::string& filename);

	public:
		/// Returns \c true if this storage contains an entry with an announcer public key matching \a announcerPublicKey.
		bool containsAnnouncer(const Key& announcerPublicKey);

	public:
		/// Adds unlocked entry pair (\a announcerPublicKey, \a encryptedEntry) associated with \a harvesterPublicKey.
		void add(const Key& announcerPublicKey, const RawBuffer& encryptedEntry, const Key& harvesterPublicKey);

		/// Removes unlocked entry identified by \a announcerPublicKey.
		void remove(const Key& announcerPublicKey);

		/// Saves unlocked entries filtered using \a filter.
		void save(const predicate<const Key&>& filter) const;

		/// Loads unlocked account entries using \a bootKeyPair and forwards to \a processKeyPair.
		void load(const crypto::KeyPair& bootKeyPair, const consumer<crypto::KeyPair&&>& processKeyPair);

	private:
		void addEntry(const Key& announcerPublicKey, const std::vector<uint8_t>& encryptedEntry, const Key& harvesterPublicKey);

		bool tryRemoveEntry(const Key& announcerPublicKey);

	private:
		std::string m_filename;
		AnnouncerToEntryMap m_announcerToEntryMap;
		EntryToHarvesterMap m_entryToHarvesterMap;
	};
}}

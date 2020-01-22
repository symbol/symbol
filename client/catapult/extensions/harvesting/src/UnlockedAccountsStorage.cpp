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

#include "UnlockedAccountsStorage.h"
#include "UnlockedFileQueueConsumer.h"
#include "catapult/crypto/AesCbcDecrypt.h"
#include "catapult/io/RawFile.h"
#include "catapult/exceptions.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace harvesting {

	namespace {
		auto GetTempFilename(const std::string& filename) {
			return filename + ".tmp";
		}

		void Append(const std::string& filename, const Key& announcerPublicKey, const RawBuffer& encryptedEntry) {
			io::RawFile output(filename, io::OpenMode::Read_Append);
			output.seek(output.size());
			output.write(announcerPublicKey);
			output.write(encryptedEntry);
		}

		void SafeAppend(const std::string& filename, const Key& announcerPublicKey, const RawBuffer& encryptedEntry) {
			auto tempFilename = GetTempFilename(filename);
			if (boost::filesystem::exists(filename))
				boost::filesystem::copy_file(filename, tempFilename, boost::filesystem::copy_option::overwrite_if_exists);

			Append(tempFilename, announcerPublicKey, encryptedEntry);
			boost::filesystem::rename(tempFilename, filename);
		}

		size_t GetLastAnnouncerPublicKey(const std::string& filename, Key& announcerPublicKey) {
			auto entrySize = Key::Size + EncryptedUnlockedEntrySize();
			io::RawFile input(filename, io::OpenMode::Read_Only);
			if (0 != input.size() % entrySize) {
				CATAPULT_LOG(warning) << filename << " is corrupt with size (" << input.size() << ")";
				CATAPULT_THROW_RUNTIME_ERROR_1("file contains incomplete entries", filename);
			}

			auto entryStartPosition = input.size() - entrySize;
			input.seek(entryStartPosition);
			input.read(announcerPublicKey);
			return entryStartPosition;
		}

		void TryRemoveLastEntry(const std::string& filename, const Key& expectedAnnouncerPublicKey) {
			if (!boost::filesystem::exists(filename)) {
				CATAPULT_LOG(warning) << filename << " does not exist";
				return;
			}

			Key announcerPublicKey;
			auto entryStartPosition = GetLastAnnouncerPublicKey(filename, announcerPublicKey);
			if (expectedAnnouncerPublicKey != announcerPublicKey) {
				CATAPULT_LOG(warning)
						<< "last announcer public key (" << announcerPublicKey << ") "
						<< "does not match expected announcer public key (" << expectedAnnouncerPublicKey << ")";
				return;
			}

			boost::filesystem::resize_file(filename, entryStartPosition);
		}
	}

	UnlockedAccountsStorage::UnlockedAccountsStorage(const std::string& filename) : m_filename(filename)
	{}

	bool UnlockedAccountsStorage::containsAnnouncer(const Key& announcerPublicKey) {
		return m_announcerToEntryMap.cend() != m_announcerToEntryMap.find(announcerPublicKey);
	}

	void UnlockedAccountsStorage::add(const Key& announcerPublicKey, const RawBuffer& encryptedEntry, const Key& harvesterPublicKey) {
		if (EncryptedUnlockedEntrySize() != encryptedEntry.Size)
			CATAPULT_THROW_INVALID_ARGUMENT_1("encrypted entry has invalid size", encryptedEntry.Size);

		if (containsAnnouncer(announcerPublicKey))
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot add same announcer public key to storage multiple times", announcerPublicKey);

		SafeAppend(m_filename, announcerPublicKey, encryptedEntry);

		std::vector<uint8_t> entry(encryptedEntry.pData, encryptedEntry.pData + encryptedEntry.Size);
		addEntry(announcerPublicKey, entry, harvesterPublicKey);
	}

	void UnlockedAccountsStorage::remove(const Key& announcerPublicKey) {
		TryRemoveLastEntry(m_filename, announcerPublicKey);

		if (!tryRemoveEntry(announcerPublicKey)) {
			CATAPULT_LOG(debug) << "cannot remove announcer public key " << announcerPublicKey << " because it is not in map";
			return;
		}
	}

	void UnlockedAccountsStorage::save(const predicate<const Key&>& filter) const {
		std::unique_ptr<io::RawFile> pRawFile;
		for (const auto& announcerEntryPair : m_announcerToEntryMap) {
			const auto& harvesterPublicKey = m_entryToHarvesterMap.find(announcerEntryPair)->second;
			if (!filter(harvesterPublicKey))
				continue;

			// delay file creation, to create file only if needed
			if (!pRawFile)
				pRawFile = std::make_unique<io::RawFile>(GetTempFilename(m_filename), io::OpenMode::Read_Write);

			pRawFile->write(announcerEntryPair.first);
			pRawFile->write(announcerEntryPair.second);
		}

		// if all entries have been filtered out, remove file
		if (!pRawFile && boost::filesystem::exists(m_filename))
			boost::filesystem::remove(m_filename);

		if (pRawFile) {
			// force file close before doing rename
			pRawFile.reset();
			boost::filesystem::rename(GetTempFilename(m_filename), m_filename);
		}
	}

	void UnlockedAccountsStorage::load(const crypto::KeyPair& bootKeyPair, const consumer<crypto::KeyPair&&>& processKeyPair) {
		if (!boost::filesystem::exists(m_filename))
			return;

		// read entries
		io::RawFile inputFile(m_filename, io::OpenMode::Read_Only);
		std::vector<uint8_t> encryptedEntry(EncryptedUnlockedEntrySize());
		while (inputFile.position() != inputFile.size()) {
			Key announcerPublicKey;
			inputFile.read(announcerPublicKey);
			inputFile.read(encryptedEntry);

			auto decryptedPair = TryDecryptUnlockedEntry(encryptedEntry, bootKeyPair);
			if (!decryptedPair.second)
				CATAPULT_THROW_RUNTIME_ERROR("malformed harvesters file");

			auto keyPair = crypto::KeyPair::FromPrivate(std::move(decryptedPair.first));
			addEntry(announcerPublicKey, encryptedEntry, keyPair.publicKey());
			processKeyPair(std::move(keyPair));
		}

		CATAPULT_LOG(info) << "loading done, closing file";
	}

	void UnlockedAccountsStorage::addEntry(
			const Key& announcerPublicKey,
			const std::vector<uint8_t>& entry,
			const Key& harvesterPublicKey) {
		auto iter = m_announcerToEntryMap.emplace(announcerPublicKey, entry).first;
		m_entryToHarvesterMap.emplace(*iter, harvesterPublicKey);
	}

	bool UnlockedAccountsStorage::tryRemoveEntry(const Key& announcerPublicKey) {
		auto iter = m_announcerToEntryMap.find(announcerPublicKey);
		if (m_announcerToEntryMap.cend() == iter)
			return false;

		m_entryToHarvesterMap.erase(*iter); // note, in this case *iter is a key for entryToHarvesterMap
		m_announcerToEntryMap.erase(iter);
		return true;
	}
}}

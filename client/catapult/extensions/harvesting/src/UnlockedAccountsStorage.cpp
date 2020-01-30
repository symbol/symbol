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

		void Append(const std::string& filename, const RawBuffer& encryptedEntry) {
			io::RawFile output(filename, io::OpenMode::Read_Append);
			output.seek(output.size());
			output.write(encryptedEntry);
		}

		void SafeAppend(const std::string& filename, const RawBuffer& encryptedEntry) {
			auto tempFilename = GetTempFilename(filename);
			if (boost::filesystem::exists(filename))
				boost::filesystem::copy_file(filename, tempFilename, boost::filesystem::copy_option::overwrite_if_exists);

			Append(tempFilename, encryptedEntry);
			boost::filesystem::rename(tempFilename, filename);
		}

		size_t GetLastMessageIdentifier(const std::string& filename, UnlockedEntryMessageIdentifier& messageIdentifier) {
			auto entrySize = EncryptedUnlockedEntrySize();
			io::RawFile input(filename, io::OpenMode::Read_Only);
			if (0 != input.size() % entrySize) {
				CATAPULT_LOG(warning) << filename << " is corrupt with size (" << input.size() << ")";
				CATAPULT_THROW_RUNTIME_ERROR_1("file contains incomplete entries", filename);
			}

			auto entryStartPosition = input.size() - entrySize;
			input.seek(entryStartPosition);
			input.read(messageIdentifier);
			return entryStartPosition;
		}

		void TryRemoveLastEntry(const std::string& filename, const UnlockedEntryMessageIdentifier& expectedMesssageIdentifier) {
			if (!boost::filesystem::exists(filename)) {
				CATAPULT_LOG(warning) << filename << " does not exist";
				return;
			}

			UnlockedEntryMessageIdentifier messageIdentifier;
			auto entryStartPosition = GetLastMessageIdentifier(filename, messageIdentifier);
			if (expectedMesssageIdentifier != messageIdentifier) {
				CATAPULT_LOG(warning)
						<< "last message identifier (" << messageIdentifier << ") "
						<< "does not match expected message identifier (" << expectedMesssageIdentifier << ")";
				return;
			}

			boost::filesystem::resize_file(filename, entryStartPosition);
		}
	}

	UnlockedAccountsStorage::UnlockedAccountsStorage(const std::string& filename) : m_filename(filename)
	{}

	bool UnlockedAccountsStorage::contains(const UnlockedEntryMessageIdentifier& messageIdentifier) {
		return m_identityToEntryMap.cend() != m_identityToEntryMap.find(messageIdentifier);
	}

	void UnlockedAccountsStorage::add(
			const UnlockedEntryMessageIdentifier& messageIdentifier,
			const RawBuffer& encryptedEntry,
			const Key& harvesterPublicKey) {
		if (EncryptedUnlockedEntrySize() != encryptedEntry.Size)
			CATAPULT_THROW_INVALID_ARGUMENT_1("encrypted entry has invalid size", encryptedEntry.Size);

		if (contains(messageIdentifier))
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot add same message identifier to storage multiple times", messageIdentifier);

		SafeAppend(m_filename, encryptedEntry);

		std::vector<uint8_t> entry(encryptedEntry.pData, encryptedEntry.pData + encryptedEntry.Size);
		addEntry(messageIdentifier, entry, harvesterPublicKey);
	}

	void UnlockedAccountsStorage::remove(const UnlockedEntryMessageIdentifier& messageIdentifier) {
		TryRemoveLastEntry(m_filename, messageIdentifier);

		if (!tryRemoveEntry(messageIdentifier)) {
			CATAPULT_LOG(debug) << "cannot remove message identifier " << messageIdentifier << " because it is not in map";
			return;
		}
	}

	void UnlockedAccountsStorage::save(const predicate<const Key&>& filter) const {
		std::unique_ptr<io::RawFile> pRawFile;
		for (const auto& identityEntryPair : m_identityToEntryMap) {
			const auto& harvesterPublicKey = m_entryToHarvesterMap.find(identityEntryPair)->second;
			if (!filter(harvesterPublicKey))
				continue;

			// delay file creation, to create file only if needed
			if (!pRawFile)
				pRawFile = std::make_unique<io::RawFile>(GetTempFilename(m_filename), io::OpenMode::Read_Write);

			pRawFile->write(identityEntryPair.second);
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
			inputFile.read(encryptedEntry);

			auto decryptedPair = TryDecryptUnlockedEntry(encryptedEntry, bootKeyPair);
			if (!decryptedPair.second)
				CATAPULT_THROW_RUNTIME_ERROR("malformed harvesters file");

			UnlockedEntryMessage message;
			message.EncryptedEntry = RawBuffer(encryptedEntry);

			auto keyPair = crypto::KeyPair::FromPrivate(std::move(decryptedPair.first));
			addEntry(GetMessageIdentifier(message), encryptedEntry, keyPair.publicKey());
			processKeyPair(std::move(keyPair));
		}

		CATAPULT_LOG(info) << "loading done, closing file";
	}

	void UnlockedAccountsStorage::addEntry(
			const UnlockedEntryMessageIdentifier& messageIdentifier,
			const std::vector<uint8_t>& entry,
			const Key& harvesterPublicKey) {
		auto iter = m_identityToEntryMap.emplace(messageIdentifier, entry).first;
		m_entryToHarvesterMap.emplace(*iter, harvesterPublicKey);
	}

	bool UnlockedAccountsStorage::tryRemoveEntry(const UnlockedEntryMessageIdentifier& messageIdentifier) {
		auto iter = m_identityToEntryMap.find(messageIdentifier);
		if (m_identityToEntryMap.cend() == iter)
			return false;

		m_entryToHarvesterMap.erase(*iter); // note, in this case *iter is a key for entryToHarvesterMap
		m_identityToEntryMap.erase(iter);
		return true;
	}
}}

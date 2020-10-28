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
#include "catapult/io/RawFile.h"
#include "catapult/exceptions.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace harvesting {

	namespace {
		auto GetTempFilename(const std::string& filename) {
			return filename + ".tmp";
		}

		void Append(const std::string& filename, const RawBuffer& encryptedPayload) {
			io::RawFile output(filename, io::OpenMode::Read_Append);
			output.seek(output.size());
			output.write(encryptedPayload);
		}

		void SafeAppend(const std::string& filename, const RawBuffer& encryptedPayload) {
			auto tempFilename = GetTempFilename(filename);
			if (boost::filesystem::exists(filename))
				boost::filesystem::copy_file(filename, tempFilename, boost::filesystem::copy_option::overwrite_if_exists);

			Append(tempFilename, encryptedPayload);
			boost::filesystem::rename(tempFilename, filename);
		}

		size_t ReadLastRequestIdentifier(const std::string& filename, HarvestRequestIdentifier& requestIdentifier) {
			auto encryptedPayloadSize = HarvestRequest::EncryptedPayloadSize();
			io::RawFile input(filename, io::OpenMode::Read_Only);
			if (0 != input.size() % encryptedPayloadSize) {
				CATAPULT_LOG(warning) << filename << " is corrupt with size (" << input.size() << ")";
				CATAPULT_THROW_RUNTIME_ERROR_1("file contains incomplete encrypted payloads", filename);
			}

			// the beginning of the encrypted payload doubles as the request identifier
			auto encryptedPayloadStartPosition = input.size() - encryptedPayloadSize;
			input.seek(encryptedPayloadStartPosition);
			input.read(requestIdentifier);
			return encryptedPayloadStartPosition;
		}

		void TryRemoveLastRequest(const std::string& filename, const HarvestRequestIdentifier& expectedRequestIdentifier) {
			if (!boost::filesystem::exists(filename)) {
				CATAPULT_LOG(warning) << filename << " does not exist";
				return;
			}

			if (0 == boost::filesystem::file_size(filename)) {
				CATAPULT_LOG(debug) << filename << " is empty";
				return;
			}

			HarvestRequestIdentifier requestIdentifier;
			auto encryptedPayloadStartPosition = ReadLastRequestIdentifier(filename, requestIdentifier);
			if (expectedRequestIdentifier != requestIdentifier) {
				CATAPULT_LOG(warning)
						<< "last request identifier (" << requestIdentifier << ") "
						<< "does not match expected request identifier (" << expectedRequestIdentifier << ")";
				return;
			}

			boost::filesystem::resize_file(filename, encryptedPayloadStartPosition);
		}
	}

	UnlockedAccountsStorage::UnlockedAccountsStorage(const std::string& filename) : m_filename(filename)
	{}

	bool UnlockedAccountsStorage::contains(const HarvestRequestIdentifier& requestIdentifier) {
		return m_identityToEncryptedPayloadMap.cend() != m_identityToEncryptedPayloadMap.find(requestIdentifier);
	}

	void UnlockedAccountsStorage::add(
			const HarvestRequestIdentifier& requestIdentifier,
			const RawBuffer& encryptedPayload,
			const Key& harvesterPublicKey) {
		if (HarvestRequest::EncryptedPayloadSize() != encryptedPayload.Size)
			CATAPULT_THROW_INVALID_ARGUMENT_1("encrypted payload has invalid size", encryptedPayload.Size);

		if (contains(requestIdentifier))
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot add same request identifier to storage multiple times", requestIdentifier);

		SafeAppend(m_filename, encryptedPayload);

		std::vector<uint8_t> encryptedPayloadCopy(encryptedPayload.pData, encryptedPayload.pData + encryptedPayload.Size);
		addRequest(requestIdentifier, encryptedPayloadCopy, harvesterPublicKey);
	}

	void UnlockedAccountsStorage::remove(const HarvestRequestIdentifier& requestIdentifier) {
		TryRemoveLastRequest(m_filename, requestIdentifier);

		if (!tryRemoveRequest(requestIdentifier)) {
			CATAPULT_LOG(debug) << "cannot remove request identifier " << requestIdentifier << " because it is not in map";
			return;
		}
	}

	void UnlockedAccountsStorage::save(const predicate<const Key&>& filter) const {
		std::unique_ptr<io::RawFile> pRawFile;
		for (const auto& request : m_identityToEncryptedPayloadMap) {
			const auto& harvesterPublicKey = m_requestToHarvesterMap.find(request)->second;
			if (!filter(harvesterPublicKey))
				continue;

			// delay file creation, to create file only if needed
			if (!pRawFile)
				pRawFile = std::make_unique<io::RawFile>(GetTempFilename(m_filename), io::OpenMode::Read_Write);

			pRawFile->write(request.second);
		}

		// if all requests have been filtered out, remove file
		if (!pRawFile && boost::filesystem::exists(m_filename))
			boost::filesystem::remove(m_filename);

		if (pRawFile) {
			// force file close before doing rename
			pRawFile.reset();
			boost::filesystem::rename(GetTempFilename(m_filename), m_filename);
		}
	}

	void UnlockedAccountsStorage::load(
			const crypto::KeyPair& encryptionKeyPair,
			const consumer<BlockGeneratorAccountDescriptor&&>& processDescriptor) {
		if (!boost::filesystem::exists(m_filename))
			return;

		// read requests
		io::RawFile inputFile(m_filename, io::OpenMode::Read_Only);
		std::vector<uint8_t> encryptedPayload(HarvestRequest::EncryptedPayloadSize());
		while (inputFile.position() != inputFile.size()) {
			inputFile.read(encryptedPayload);

			auto decryptedPair = TryDecryptBlockGeneratorAccountDescriptor(encryptedPayload, encryptionKeyPair);
			if (!decryptedPair.second)
				CATAPULT_THROW_RUNTIME_ERROR("malformed harvesters file");

			HarvestRequest request;
			request.EncryptedPayload = RawBuffer(encryptedPayload);

			auto& descriptor = decryptedPair.first;
			addRequest(GetRequestIdentifier(request), encryptedPayload, descriptor.signingKeyPair().publicKey());
			processDescriptor(std::move(descriptor));
		}

		CATAPULT_LOG(info) << "loading done, closing file";
	}

	void UnlockedAccountsStorage::addRequest(
			const HarvestRequestIdentifier& requestIdentifier,
			const std::vector<uint8_t>& encryptedPayload,
			const Key& harvesterPublicKey) {
		auto iter = m_identityToEncryptedPayloadMap.emplace(requestIdentifier, encryptedPayload).first;
		m_requestToHarvesterMap.emplace(*iter, harvesterPublicKey);
	}

	bool UnlockedAccountsStorage::tryRemoveRequest(const HarvestRequestIdentifier& requestIdentifier) {
		auto iter = m_identityToEncryptedPayloadMap.find(requestIdentifier);
		if (m_identityToEncryptedPayloadMap.cend() == iter)
			return false;

		m_requestToHarvesterMap.erase(*iter); // note, in this case *iter is a key for requestToHarvesterMap
		m_identityToEncryptedPayloadMap.erase(iter);
		return true;
	}
}}

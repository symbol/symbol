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

#include "TransactionInfoSerializer.h"
#include "EntityIoUtils.h"
#include "Stream.h"

namespace catapult { namespace io {

	void WriteTransactionInfo(const model::TransactionInfo& transactionInfo, OutputStream& outputStream) {
		outputStream.write(transactionInfo.EntityHash);
		outputStream.write(transactionInfo.MerkleComponentHash);

		if (transactionInfo.OptionalExtractedAddresses) {
			Write64(outputStream, transactionInfo.OptionalExtractedAddresses->size());
			for (const auto& address : *transactionInfo.OptionalExtractedAddresses)
				outputStream.write({ reinterpret_cast<const uint8_t*>(address.data()), address.size() });
		} else {
			Write64(outputStream, std::numeric_limits<uint64_t>::max());
		}

		WriteEntity(outputStream, *transactionInfo.pEntity);
	}

	void ReadTransactionInfo(InputStream& inputStream, model::TransactionInfo& transactionInfo) {
		inputStream.read(transactionInfo.EntityHash);
		inputStream.read(transactionInfo.MerkleComponentHash);

		auto numAddresses = Read64(inputStream);
		if (std::numeric_limits<uint64_t>::max() != numAddresses) {
			UnresolvedAddress address;
			auto pExtractedAddresses = std::make_shared<model::UnresolvedAddressSet>();
			for (auto i = 0u; i < numAddresses; ++i) {
				inputStream.read({ reinterpret_cast<uint8_t*>(address.data()), address.size() });
				pExtractedAddresses->insert(address);
			}

			transactionInfo.OptionalExtractedAddresses = pExtractedAddresses;
		}

		transactionInfo.pEntity = ReadEntity<model::Transaction>(inputStream);
	}

	void WriteTransactionInfos(const model::TransactionInfosSet& transactionInfos, OutputStream& outputStream) {
		Write32(outputStream, static_cast<uint32_t>(transactionInfos.size()));
		for (const auto& transactionInfo : transactionInfos)
			WriteTransactionInfo(transactionInfo, outputStream);
	}

	void ReadTransactionInfos(InputStream& inputStream, model::TransactionInfosSet& transactionInfos) {
		auto numTransactionInfos = Read32(inputStream);
		for (auto i = 0u; i < numTransactionInfos; ++i) {
			model::TransactionInfo transactionInfo;
			ReadTransactionInfo(inputStream, transactionInfo);
			transactionInfos.insert(std::move(transactionInfo));
		}
	}
}}

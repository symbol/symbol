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

#include "BlockElementSerializer.h"
#include "PodIoUtils.h"
#include "Stream.h"
#include "catapult/utils/MemoryUtils.h"

namespace catapult { namespace io {

	// region WriteBlockElement

	namespace {
		void WriteTransactionHashes(OutputStream& outputStream, const std::vector<model::TransactionElement>& transactionElements) {
			auto numTransactions = static_cast<uint32_t>(transactionElements.size());
			Write32(outputStream, numTransactions);
			std::vector<Hash256> hashes(2 * numTransactions);
			auto iter = hashes.begin();
			for (const auto& transactionElement : transactionElements) {
				*iter++ = transactionElement.EntityHash;
				*iter++ = transactionElement.MerkleComponentHash;
			}

			outputStream.write({ reinterpret_cast<const uint8_t*>(hashes.data()), hashes.size() * Hash256::Size });
		}

		void WriteSubCacheMerkleRoots(OutputStream& outputStream, const std::vector<Hash256>& subCacheMerkleRoots) {
			auto numHashes = static_cast<uint32_t>(subCacheMerkleRoots.size());
			Write32(outputStream, numHashes);
			outputStream.write({ reinterpret_cast<const uint8_t*>(subCacheMerkleRoots.data()), numHashes * Hash256::Size });
		}
	}

	void WriteBlockElement(const model::BlockElement& blockElement, OutputStream& outputStream) {
		// 1. write constant size data
		outputStream.write({ reinterpret_cast<const uint8_t*>(&blockElement.Block), blockElement.Block.Size });
		outputStream.write(blockElement.EntityHash);
		outputStream.write(blockElement.GenerationHash);

		// 2. write transaction hashes
		WriteTransactionHashes(outputStream, blockElement.Transactions);

		// 3. write sub cache merkle roots
		WriteSubCacheMerkleRoots(outputStream, blockElement.SubCacheMerkleRoots);
	}

	// endregion

	// region ReadBlockElement

	namespace {
		std::shared_ptr<model::BlockElement> ReadBlockElementImpl(InputStream& inputStream) {
			// allocate memory for both the element and the block in one shot (Block data is appended)
			auto size = Read32(inputStream);
			auto pBackingMemory = utils::MakeUniqueWithSize<uint8_t>(sizeof(model::BlockElement) + size);

			// read the block data
			auto pBlockData = pBackingMemory.get() + sizeof(model::BlockElement);
			reinterpret_cast<uint32_t&>(*pBlockData) = size;
			inputStream.read({ pBlockData + sizeof(uint32_t), size - sizeof(uint32_t) });

			// create the block element and transfer ownership from pBackingMemory to pBlockElement
			auto pBlockElementRaw = new (pBackingMemory.get()) model::BlockElement(*reinterpret_cast<model::Block*>(pBlockData));
			auto pBlockElement = std::shared_ptr<model::BlockElement>(pBlockElementRaw);
			pBackingMemory.release();

			// read metadata
			inputStream.read(pBlockElement->EntityHash);
			inputStream.read(pBlockElement->GenerationHash);
			return pBlockElement;
		}

		void ReadTransactionHashes(InputStream& inputStream, model::BlockElement& blockElement) {
			auto numTransactions = Read32(inputStream);
			std::vector<Hash256> hashes(2 * numTransactions);
			inputStream.read({ reinterpret_cast<uint8_t*>(hashes.data()), hashes.size() * Hash256::Size });

			size_t i = 0;
			for (const auto& transaction : blockElement.Block.Transactions()) {
				blockElement.Transactions.push_back(model::TransactionElement(transaction));
				blockElement.Transactions.back().EntityHash = hashes[i++];
				blockElement.Transactions.back().MerkleComponentHash = hashes[i++];
			}
		}

		void ReadSubCacheMerkleRoots(InputStream& inputStream, std::vector<Hash256>& subCacheMerkleRoots) {
			auto numHashes = Read32(inputStream);
			subCacheMerkleRoots.resize(numHashes);
			inputStream.read({ reinterpret_cast<uint8_t*>(subCacheMerkleRoots.data()), numHashes * Hash256::Size });
		}
	}

	std::shared_ptr<model::BlockElement> ReadBlockElement(InputStream& inputStream) {
		auto pBlockElement = ReadBlockElementImpl(inputStream);
		ReadTransactionHashes(inputStream, *pBlockElement);
		ReadSubCacheMerkleRoots(inputStream, pBlockElement->SubCacheMerkleRoots);
		return pBlockElement;
	}

	// endregion
}}

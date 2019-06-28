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

#include "MemoryBlockStorage.h"
#include "catapult/io/BlockStatementSerializer.h"
#include "catapult/io/Stream.h"
#include "catapult/utils/MemoryUtils.h"

namespace catapult { namespace extensions {

	// region ctor

	MemoryBlockStorage::MemoryBlockStorage(const model::BlockElement& nemesisBlockElement) {
		saveBlock(nemesisBlockElement);
	}

	// endregion

	// region LightBlockStorage

	Height MemoryBlockStorage::chainHeight() const {
		return m_height;
	}

	model::HashRange MemoryBlockStorage::loadHashesFrom(Height height, size_t maxHashes) const {
		auto currentHeight = chainHeight();
		if (Height(0) == height || currentHeight < height)
			return model::HashRange();

		auto numAvailableHashes = static_cast<size_t>((currentHeight - height).unwrap() + 1);
		auto numHashes = std::min(maxHashes, numAvailableHashes);

		auto range = model::HashRange::PrepareFixed(numHashes);
		auto rangeIter = range.begin();
		for (auto i = 0u; i < numHashes; ++i)
			*rangeIter++ = m_blockElements.find(height + Height(i))->second->EntityHash;

		return range;
	}

	namespace {
		void CopyHashes(model::TransactionElement& destElement, const model::TransactionElement& srcElement) {
			destElement.EntityHash = srcElement.EntityHash;
			destElement.MerkleComponentHash = srcElement.MerkleComponentHash;
		}

		std::unique_ptr<model::Block> CopyBlock(const model::Block& block) {
			auto pBlock = utils::MakeUniqueWithSize<model::Block>(block.Size);
			std::memcpy(static_cast<void*>(pBlock.get()), &block, block.Size);
			return pBlock;
		}

		std::shared_ptr<model::BlockElement> Copy(const model::Block& block, const model::BlockElement& blockElement) {
			auto pElement = std::make_shared<model::BlockElement>(block);
			pElement->EntityHash = blockElement.EntityHash;
			pElement->GenerationHash = blockElement.GenerationHash;
			pElement->SubCacheMerkleRoots = blockElement.SubCacheMerkleRoots;

			auto i = 0u;
			for (const auto& transaction : block.Transactions()) {
				pElement->Transactions.emplace_back(model::TransactionElement(transaction));
				CopyHashes(pElement->Transactions.back(), blockElement.Transactions[i]);
				++i;
			}

			if (blockElement.OptionalStatement) {
				auto pBlockStatement = std::make_shared<model::BlockStatement>();
				DeepCopyTo(*pBlockStatement, *blockElement.OptionalStatement);
				pElement->OptionalStatement = std::move(pBlockStatement);
			}

			return pElement;
		}
	}

	void MemoryBlockStorage::saveBlock(const model::BlockElement& blockElement) {
		auto height = blockElement.Block.Height;
		if (height != m_height + Height(1)) {
			std::ostringstream out;
			out << "cannot save block with height " << height << " when storage height is " << m_height;
			CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
		}

		m_blocks[height] = CopyBlock(blockElement.Block);

		// simulate file storage, which stores elements and statements separately
		m_blockElements[height] = Copy(*m_blocks[height], blockElement);
		m_blockStatements[height] = m_blockElements[height]->OptionalStatement;
		m_blockElements[height]->OptionalStatement.reset();

		m_height = std::max(m_height, height);
	}

	void MemoryBlockStorage::dropBlocksAfter(Height height) {
		m_height = height;
	}

	// endregion

	// region BlockStorage

	namespace {
		[[noreturn]]
		void ThrowInvalidHeight(Height height) {
			std::ostringstream out;
			out << "block not found at height: " << height;
			CATAPULT_THROW_FILE_IO_ERROR(out.str().c_str());
		}
	}

	std::shared_ptr<const model::Block> MemoryBlockStorage::loadBlock(Height height) const {
		requireHeight(height, "block");
		auto iter = m_blocks.find(height);
		if (m_blocks.end() == iter)
			ThrowInvalidHeight(height);

		return iter->second;
	}

	std::shared_ptr<const model::BlockElement> MemoryBlockStorage::loadBlockElement(Height height) const {
		requireHeight(height, "block element");
		auto iter = m_blockElements.find(height);
		if (m_blockElements.end() == iter)
			ThrowInvalidHeight(height);

		return iter->second;
	}

	namespace {
		class BufferOutputStream : public io::OutputStream {
		public:
			explicit BufferOutputStream(std::vector<uint8_t>& buffer) : m_buffer(buffer)
			{}

		public:
			void write(const RawBuffer& buffer) override {
				m_buffer.insert(m_buffer.end(), buffer.pData, buffer.pData + buffer.Size);
			}

			void flush() override
			{}

		private:
			std::vector<uint8_t>& m_buffer;
		};
	}

	std::pair<std::vector<uint8_t>, bool> MemoryBlockStorage::loadBlockStatementData(Height height) const {
		requireHeight(height, "block statement data");
		auto pBlockStatement = m_blockStatements.find(height)->second; // throw if not found
		if (!pBlockStatement)
			return std::make_pair(std::vector<uint8_t>(), false);

		std::vector<uint8_t> serialized;
		BufferOutputStream stream(serialized);
		io::WriteBlockStatement(*pBlockStatement, stream);
		return std::make_pair(std::move(serialized), true);
	}

	// endregion

	// region PrunableBlockStorage

	void MemoryBlockStorage::purge() {
		m_blocks.clear();
		m_blockElements.clear();
		m_blockStatements.clear();
		m_height = Height(0);
	}

	// endregion

	// region requireHeight

	void MemoryBlockStorage::requireHeight(Height height, const char* description) const {
		if (height <= m_height)
			return;

		std::ostringstream out;
		out << "cannot load " << description << " at height (" << height << ") greater than chain height (" << m_height << ")";
		CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
	}

	// endregion
}}

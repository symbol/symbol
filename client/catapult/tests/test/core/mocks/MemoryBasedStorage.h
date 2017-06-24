#pragma once
#include "catapult/io/BlockStorage.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/model/Elements.h"
#include <map>

namespace catapult { namespace mocks {

	extern const unsigned char MemoryBasedStorage_NemesisBlockData[];

	/// A mock memory-based BlockStorage that loads and saves blocks in memory.
	class MemoryBasedStorage final : public io::BlockStorage {
	private:
		using Blocks = std::map<Height, std::shared_ptr<model::Block>>;
		using BlockElements = std::map<Height, std::shared_ptr<model::BlockElement>>;

	public:
		/// Creates a memory based block storage.
		MemoryBasedStorage();

	public:
		Height chainHeight() const override;

	public:
		std::shared_ptr<const model::Block> loadBlock(Height height) const override;
		std::shared_ptr<const model::BlockElement> loadBlockElement(Height height) const override;
		model::HashRange loadHashesFrom(Height height, size_t maxHashes) const override;

		void saveBlock(const model::BlockElement& blockElement) override;
		void dropBlocksAfter(Height height) override;

	private:
		Blocks m_blocks;
		BlockElements m_blockElements;
		Height m_height;
	};

	/// Creates a memory based storage composed of \a numBlocks.
	std::unique_ptr<io::BlockStorage> CreateMemoryBasedStorage(uint32_t numBlocks);

	/// Creates a memory based storage cache composed of \a numBlocks.
	std::unique_ptr<io::BlockStorageCache> CreateMemoryBasedStorageCache(uint32_t numBlocks);
}}

#pragma once
#include "BlockStorage.h"
#include "RawFile.h"
#include <mutex>
#include <string>

namespace catapult { namespace io {

	/// File-based block storage.
	class FileBasedStorage final : public PrunableBlockStorage {
	public:
		/// Creates a file-based storage, where blocks will be stored inside \a dataDirectory.
		explicit FileBasedStorage(const std::string& dataDirectory);

	public:
		Height chainHeight() const override;

	public:
		std::shared_ptr<const model::Block> loadBlock(Height height) const override;
		std::shared_ptr<const model::BlockElement> loadBlockElement(Height height) const override;

		model::HashRange loadHashesFrom(Height height, size_t maxHashes) const override;

		void saveBlock(const model::BlockElement& blockElement) override;
		void dropBlocksAfter(Height height) override;

	public:
		void pruneBlocksBefore(Height height) override;

	private:
		class HashFile final {
		public:
			explicit HashFile(const std::string& dataDirectory);

			model::HashRange loadHashesFrom(Height height, size_t numHashes) const;
			void save(Height height, const Hash256& hash);

		private:
			const std::string& m_dataDirectory;

			// used for caching inside save()
			uint64_t m_cachedDirectoryId;
			std::unique_ptr<RawFile> m_pCachedHashFile;
		};

		std::string m_dataDirectory;
		HashFile m_hashFile;
	};
}}

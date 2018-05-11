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
#include "BlockStorage.h"
#include "RawFile.h"
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

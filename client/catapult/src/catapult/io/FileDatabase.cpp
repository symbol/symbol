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

#include "FileDatabase.h"
#include "FileStream.h"
#include "PodIoUtils.h"
#include "catapult/exceptions.h"
#include "catapult/preprocessor.h"

namespace catapult { namespace io {

	namespace {
		// region InputStreamSlice

		class InputStreamSlice : public InputStream {
		public:
			InputStreamSlice(std::unique_ptr<SeekableStream>&& pStream, size_t endPosition)
					: m_pStream(std::move(pStream))
					, m_endPosition(endPosition)
			{}

		public:
			bool eof() const override {
				return m_pStream->position() == m_endPosition;
			}

			void read(const MutableRawBuffer& buffer) override {
				if (buffer.Size + m_pStream->position() > m_endPosition) {
					std::ostringstream out;
					out
							<< "InputStreamSlice invalid read (read-size = " << buffer.Size
							<< ", stream-position = " << m_pStream->position()
							<< ", stream-size = " << m_endPosition << ")";
					CATAPULT_THROW_FILE_IO_ERROR(out.str().c_str());
				}

				m_pStream->read(buffer);
			}

		private:
			std::unique_ptr<SeekableStream> m_pStream;
			size_t m_endPosition;
		};

		// endregion
	}

	// region FileDatabase

	FileDatabase::FileDatabase(const config::CatapultDirectory& directory, const Options& options)
			: m_directory(directory)
			, m_options(options) {
		if (0 == m_options.BatchSize)
			CATAPULT_THROW_INVALID_ARGUMENT("batch size must be nonzero");
	}

	bool FileDatabase::contains(uint64_t id) const {
		auto filePath = getFilePath(id, false);
		if (!std::filesystem::exists(filePath))
			return false;

		if (bypassHeader())
			return true;

		auto rawFile = RawFile(filePath, OpenMode::Read_Only);

		auto headerOffset = getHeaderOffset(id);
		rawFile.seek(headerOffset);
		return 0 != Read64(rawFile);
	}

	std::unique_ptr<InputStream> FileDatabase::inputStream(uint64_t id, size_t* pSize) const {
		auto filePath = getFilePath(id, false);

		auto rawFile = RawFile(filePath, OpenMode::Read_Only);
		auto rawFileSize = rawFile.size();

		if (bypassHeader()) {
			if (pSize)
				*pSize = rawFileSize;

			return std::make_unique<FileStream>(std::move(rawFile));
		}

		auto headerOffset = getHeaderOffset(id);
		rawFile.seek(headerOffset);
		auto bodyStartOffset = Read64(rawFile);

		if (0 == bodyStartOffset) {
			std::ostringstream out;
			out << "cannot read payload at " << id << " that has not been written";
			CATAPULT_THROW_FILE_IO_ERROR(out.str().c_str());
		}

		uint64_t bodyEndOffset = 0;
		if (m_options.BatchSize - 1 != id % m_options.BatchSize)
			bodyEndOffset = Read64(rawFile);

		auto pBodyStream = std::make_unique<FileStream>(std::move(rawFile));
		pBodyStream->seek(bodyStartOffset);
		if (0 == bodyEndOffset) // payload extends to end of file
			bodyEndOffset = rawFileSize;

		if (pSize)
			*pSize = bodyEndOffset - pBodyStream->position();

		return std::make_unique<InputStreamSlice>(std::move(pBodyStream), bodyEndOffset);
	}

	std::unique_ptr<OutputStream> FileDatabase::outputStream(uint64_t id) {
		auto filePath = getFilePath(id, true);

		auto isNewFile = !std::filesystem::exists(filePath) || bypassHeader();
		auto rawFile = RawFile(filePath, isNewFile ? OpenMode::Read_Write : OpenMode::Read_Append);

		if (bypassHeader())
			return std::make_unique<FileStream>(std::move(rawFile));

		auto headerOffset = getHeaderOffset(id);
		auto headerSize = m_options.BatchSize * sizeof(uint64_t);
		if (isNewFile) {
			// preallocate index header
			rawFile.write(std::vector<uint8_t>(headerSize));
		} else {
			// seek to header offset
			rawFile.seek(headerOffset);

			// if this payload has already been written, need to clear any indexes after it
			auto bodyStartOffset = Read64(rawFile);
			if (0 != bodyStartOffset) {
				rawFile.seek(bodyStartOffset);
				rawFile.truncate();

				// clear offsets >= id
				rawFile.seek(headerOffset);
				rawFile.write(std::vector<uint8_t>(headerSize - headerOffset));
			}
		}

		// update the header
		rawFile.seek(headerOffset);
		Write64(rawFile, rawFile.size());

		// seek to the body and return
		rawFile.seek(rawFile.size());
		return std::make_unique<FileStream>(std::move(rawFile));
	}

	bool FileDatabase::bypassHeader() const {
		// skip header when batch size is one to preserve old behavior
		return 1 == m_options.BatchSize;
	}

	uint64_t FileDatabase::getHeaderOffset(uint64_t id) const {
		return id % m_options.BatchSize * sizeof(uint64_t);
	}

	std::string FileDatabase::getFilePath(uint64_t id, bool createDirectories) const {
		struct GroupIdentifier_tag {};
		using GroupIdentifier = utils::BaseValue<uint64_t, GroupIdentifier_tag>;

		GroupIdentifier groupId((id / m_options.BatchSize) * m_options.BatchSize);
		auto storageDirectory = config::CatapultDataDirectory(m_directory.path()).storageDir(groupId);

		if (createDirectories)
			config::CatapultDirectory(storageDirectory.str()).create();

		return storageDirectory.storageFile(m_options.FileExtension);
	}

	// endregion
}}

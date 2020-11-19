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

#include "FileQueue.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/utils/HexFormatter.h"
#include "catapult/exceptions.h"
#include <filesystem>
#include <sstream>

namespace catapult { namespace io {

	namespace {
		const std::filesystem::path& CreateDirectory(const std::filesystem::path& directory) {
			config::CatapultDirectory(directory).create();
			return directory;
		}

		bool CreateIfNotExists(IndexFile& indexFile) {
			if (indexFile.exists())
				return false;

			indexFile.set(0);
			return true;
		}

		std::string GetFilename(uint64_t value) {
			std::ostringstream out;
			out << utils::HexFormat(value) << ".dat";
			return out.str();
		}
	}

	// region FileQueueWriter

	FileQueueWriter::FileQueueWriter(const std::string& directory) : FileQueueWriter(directory, "index.dat")
	{}

	FileQueueWriter::FileQueueWriter(const std::string& directory, const std::string& indexFilename)
			: m_directory(CreateDirectory(directory))
			, m_indexFile((m_directory / indexFilename).generic_string(), LockMode::None)
			, m_indexValue(CreateIfNotExists(m_indexFile) ? 0 : m_indexFile.get())
	{}

	void FileQueueWriter::write(const RawBuffer& buffer) {
		if (!m_pOutputStream) {
			auto filename = (m_directory / GetFilename(m_indexValue)).generic_string();
			RawFile outputFile(filename, OpenMode::Read_Write);
			m_pOutputStream = std::make_unique<BufferedOutputFileStream>(std::move(outputFile));
		}

		m_pOutputStream->write(buffer);
	}

	void FileQueueWriter::flush() {
		if (!m_pOutputStream)
			return;

		m_pOutputStream->flush();
		m_pOutputStream.reset();
		m_indexValue = m_indexFile.increment();
	}

	// endregion

	// region FileQueueReader

	namespace {
		std::vector<uint8_t> ReadAllContents(const std::string& filename) {
			RawFile outputFile(filename, OpenMode::Read_Only);
			std::vector<uint8_t> buffer(outputFile.size());
			outputFile.read(buffer);
			return buffer;
		}
	}

	FileQueueReader::FileQueueReader(const std::string& directory) : FileQueueReader(directory, "index_reader.dat", "index.dat")
	{}

	FileQueueReader::FileQueueReader(
			const std::string& directory,
			const std::string& readerIndexFilename,
			const std::string& writerIndexFilename)
			: m_directory(CreateDirectory(directory))
			, m_readerIndexFile((m_directory / readerIndexFilename).generic_string())
			, m_writerIndexFile((m_directory / writerIndexFilename).generic_string(), LockMode::None) {
		CreateIfNotExists(m_readerIndexFile);
	}

	size_t FileQueueReader::pending() const {
		auto writerIndexValue = m_writerIndexFile.exists() ? m_writerIndexFile.get() : 0;
		auto readerIndexValue = m_readerIndexFile.get();
		return readerIndexValue > writerIndexValue ? 0 : writerIndexValue - readerIndexValue;
	}

	bool FileQueueReader::tryReadNextMessage(const consumer<const std::vector<uint8_t>&>& consumer) {
		return process([consumer](const auto& nextMessageFilename) {
			auto buffer = ReadAllContents(nextMessageFilename);
			consumer(buffer);
		});
	}

	void FileQueueReader::skip(uint32_t count) {
		for (auto i = 0u; i < count; ++i)
			process([](const auto&) {});
	}

	bool FileQueueReader::process(const consumer<const std::string&>& processFilename) {
		auto readerIndexValue = m_readerIndexFile.get();
		if (!m_writerIndexFile.exists() || readerIndexValue >= m_writerIndexFile.get())
			return false;

		auto nextMessageFilename = m_directory / GetFilename(readerIndexValue);
		if (!std::filesystem::exists(nextMessageFilename))
			CATAPULT_THROW_RUNTIME_ERROR_1("reading from file queue failed due to missing message file", nextMessageFilename);

		processFilename(nextMessageFilename.generic_string());

		m_readerIndexFile.increment();
		std::filesystem::remove(nextMessageFilename);
		return true;
	}

	// endregion
}}

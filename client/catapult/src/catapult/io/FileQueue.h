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

#pragma once
#include "BufferedFileStream.h"
#include "IndexFile.h"
#include "catapult/functions.h"
#include <filesystem>

namespace catapult { namespace io {

	/// File based queue writer where each message is represented by a file (with incrementing names) in a directory.
	/// \note Each call to flush will additionally create a new file.
	class FileQueueWriter final : public OutputStream {
	public:
		/// Creates a file queue writer around \a directory.
		explicit FileQueueWriter(const std::string& directory);

		/// Creates a file queue writer around \a directory containing a (writer) index file (\a indexFilename).
		FileQueueWriter(const std::string& directory, const std::string& indexFilename);

	public:
		void write(const RawBuffer& buffer) override;
		void flush() override;

	private:
		std::filesystem::path m_directory;
		IndexFile m_indexFile;
		uint64_t m_indexValue;
		std::unique_ptr<BufferedOutputFileStream> m_pOutputStream;
	};

	/// File based queue reader where each message is represented by a file (with incrementing names) in a directory.
	class FileQueueReader final {
	public:
		/// Creates a file queue reader around \a directory.
		explicit FileQueueReader(const std::string& directory);

		/// Creates a file queue reader around \a directory containing (reader and writer) index files
		/// (\a readerIndexFilename, \a writerIndexFilename).
		FileQueueReader(const std::string& directory, const std::string& readerIndexFilename, const std::string& writerIndexFilename);

	public:
		/// Gets the number of pending messages.
		size_t pending() const;

	public:
		/// Tries to read the next message and forwards it to \a consumer if successful.
		bool tryReadNextMessage(const consumer<const std::vector<uint8_t>&>& consumer);

		/// Skips at most the next \a count messages.
		void skip(uint32_t count);

	private:
		bool process(const consumer<const std::string&>& processFilename);

	private:
		std::filesystem::path m_directory;
		IndexFile m_readerIndexFile;
		IndexFile m_writerIndexFile;
	};
}}

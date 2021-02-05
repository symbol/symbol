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
#include "Stream.h"
#include "catapult/config/CatapultDataDirectory.h"

namespace catapult { namespace io {

	/// Database that stores arbitrary payloads indexed by ids across multiple files.
	class FileDatabase {
	public:
		/// File database options.
		struct Options {
			/// Maximum number of payloads per file.
			size_t BatchSize;

			/// Extension of created files.
			std::string FileExtension;
		};

	public:
		/// Creates a database in \a directory with \a options.
		FileDatabase(const config::CatapultDirectory& directory, const Options& options);

	public:
		/// Returns \c true if a payload for \a id is contained.
		bool contains(uint64_t id) const;

		/// Gets an input stream for \a id and optionally returns the stream size (\a pSize).
		std::unique_ptr<InputStream> inputStream(uint64_t id, size_t* pSize = nullptr) const;

		/// Gets an output stream for \a id.
		std::unique_ptr<OutputStream> outputStream(uint64_t id);

	private:
		bool bypassHeader() const;
		uint64_t getHeaderOffset(uint64_t id) const;
		std::string getFilePath(uint64_t id, bool createDirectories) const;

	private:
		config::CatapultDirectory m_directory;
		Options m_options;
	};
}}

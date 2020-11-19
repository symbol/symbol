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

#include "IndexFile.h"
#include "PodIoUtils.h"
#include <filesystem>

namespace catapult { namespace io {

	IndexFile::IndexFile(const std::string& filename, LockMode lockMode)
			: m_filename(filename)
			, m_lockMode(lockMode)
	{}

	bool IndexFile::exists() const {
		return std::filesystem::is_regular_file(m_filename);
	}

	uint64_t IndexFile::get() const {
		auto indexFile = open(OpenMode::Read_Only);
		return 8 == indexFile.size() ? Read64(indexFile) : 0;
	}

	void IndexFile::set(uint64_t value) {
		auto indexFile = open(OpenMode::Read_Append);
		indexFile.seek(0);
		Write64(indexFile, value);
	}

	uint64_t IndexFile::increment() {
		if (!exists()) {
			set(0);
			return 0;
		}

		auto indexFile = open(OpenMode::Read_Append);
		auto value = Read64(indexFile);
		++value;

		indexFile.seek(0);
		Write64(indexFile, value);
		return value;
	}

	RawFile IndexFile::open(OpenMode mode) const {
		return RawFile(m_filename, mode, m_lockMode);
	}
}}

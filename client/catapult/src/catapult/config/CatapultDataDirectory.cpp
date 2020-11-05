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

#include "CatapultDataDirectory.h"
#include "catapult/exceptions.h"

#ifndef _WIN32
#include <sys/stat.h>
#include <sys/types.h>
#endif

namespace catapult { namespace config {

	namespace {
#ifdef _WIN32
		bool CreateDirectory(const boost::filesystem::path& directory) {
			boost::system::error_code ec;
			boost::filesystem::create_directory(directory, ec);
			return !ec;
		}
#else
		bool CreateDirectory(const boost::filesystem::path& directory) {
			return !mkdir(directory.generic_string().c_str(), 0700);
		}
#endif
	}

	bool CatapultDirectory::exists() const {
		return boost::filesystem::is_directory(m_directory);
	}

	void CatapultDirectory::create() const {
		if (exists())
			return;

		if (!CreateDirectory(m_directory))
			CATAPULT_THROW_RUNTIME_ERROR_1("couldn't create directory", m_directory.generic_string());
	}

	void CatapultDirectory::createAll() const {
		boost::filesystem::path currentDirectory;

		for (const auto& part : m_directory) {
			if (currentDirectory.empty())
				currentDirectory = part;
			else
				currentDirectory /= part;

			CatapultDirectory(currentDirectory).create();
		}
	}
}}

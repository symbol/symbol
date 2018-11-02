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

#ifdef __clang__
// workaround for https://llvm.org/bugs/show_bug.cgi?id=25230
#pragma GCC visibility push(default)
#include <string>
#pragma GCC visibility pop
#endif

#include "Filesystem.h"
#include "catapult/utils/Logging.h"
#include "catapult/exceptions.h"
#include <boost/filesystem.hpp>

#ifdef CATAPULT_DOCKER_TESTS
extern int global_argc;
extern char** global_argv;
#endif

namespace catapult { namespace test {

	// region directory guard

	TempDirectoryGuard::TempDirectoryGuard(const std::string& directoryName) : m_directoryPath(directoryName) {
		if (exists())
			CATAPULT_LOG(warning) << "creating directory " << m_directoryPath << " that already exists!";
		else
			CATAPULT_LOG(debug) << "creating directory " << m_directoryPath;

		boost::filesystem::create_directories(m_directoryPath);
	}

	TempDirectoryGuard::~TempDirectoryGuard() {
		auto numRemovedFiles = boost::filesystem::remove_all(m_directoryPath);
		CATAPULT_LOG(debug)
				<< "deleted directory " << m_directoryPath << " and removed " << numRemovedFiles
				<< " files (exists? " << exists() << ")";
	}

	std::string TempDirectoryGuard::name() const {
		return m_directoryPath.generic_string();
	}

	bool TempDirectoryGuard::exists() const {
		return boost::filesystem::exists(m_directoryPath);
	}

	// endregion

	// region file guard

	TempFileGuard::TempFileGuard(const std::string& name) : m_name(name)
	{}

	TempFileGuard::~TempFileGuard() {
		remove(m_name.c_str());
	}

	const std::string& TempFileGuard::name() const {
		return m_name;
	}

	// endregion

	// region GetExplicitPluginsDirectory

	namespace {
		bool TryFindPluginsDirectory(const std::string& directory, bool recurse, std::string& pluginsDirectory) {
			if (!boost::filesystem::is_directory(directory))
				return false;

			using boost::filesystem::directory_iterator;
			for (auto iter = directory_iterator(directory); directory_iterator() != iter; ++iter) {
				if (std::string::npos != iter->path().generic_string().find("catapult.plugins")) {
					pluginsDirectory = directory;
					return true;
				}

				if (!recurse || !boost::filesystem::is_directory(iter->path()))
					continue;

				if (TryFindPluginsDirectory(iter->path().generic_string(), false, pluginsDirectory))
					return true;
			}

			return false;
		}
	}

	std::string GetExplicitPluginsDirectory() {
#ifdef CATAPULT_DOCKER_TESTS
		for (auto i = 0; i < global_argc; ++i) {
			std::string pluginsDirectory;
			if (TryFindPluginsDirectory(global_argv[i], true, pluginsDirectory)) {
				CATAPULT_LOG(debug) << "selecting '" << pluginsDirectory << "' as explicit directory";
				return pluginsDirectory;
			}
		}
#endif

		for (const auto& directory : { "bin", "." }) {
			std::string pluginsDirectory;
			if (TryFindPluginsDirectory(directory, true, pluginsDirectory)) {
				CATAPULT_LOG(debug) << "selecting '" << pluginsDirectory << "' as explicit directory";
				return pluginsDirectory;
			}
		}

		CATAPULT_THROW_RUNTIME_ERROR("unable to find suitable explicit directory");
	}

	// endregion
}}

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

#include "Filesystem.h"
#include "catapult/utils/Logging.h"
#include "catapult/exceptions.h"
#include <boost/dll.hpp>
#include <filesystem>

#ifdef CATAPULT_DOCKER_TESTS
extern int global_argc;
extern char** global_argv;
#endif

namespace catapult { namespace test {

	// region directory guard

	namespace {
		constexpr auto Temp_Directory_Root = "../_temp";

		std::filesystem::path PathFromDirectoryName(const std::string& directoryName) {
			if (std::string::npos != directoryName.find("/"))
				CATAPULT_THROW_INVALID_ARGUMENT_1("TempDirectoryGuard only supports directory names", directoryName);

			return std::filesystem::path(TempDirectoryGuard::DefaultName()) / directoryName;
		}

		void DeleteDirectoryIfEmpty(const std::string& directoryName) {
			if (!std::filesystem::is_directory(directoryName))
				return;

			auto begin = std::filesystem::directory_iterator(directoryName);
			auto end = std::filesystem::directory_iterator();
			auto numFiles = std::distance(begin, end);
			if (0 != numFiles)
				return;

			CATAPULT_LOG(debug) << "deleting empty directory " << directoryName;
			std::filesystem::remove_all(directoryName);
		}

		bool StartsWithTempDirectoryRoot(const std::string& directoryName) {
			return directoryName == Temp_Directory_Root || 0 == directoryName.find(Temp_Directory_Root + std::string("/"));
		}
	}

	TempDirectoryGuard::TempDirectoryGuard() : TempDirectoryGuard(DefaultName(), true)
	{}

	TempDirectoryGuard::TempDirectoryGuard(const std::string& directoryName)
			: TempDirectoryGuard(PathFromDirectoryName(directoryName), true)
	{}

	TempDirectoryGuard::~TempDirectoryGuard() {
		auto numRemovedFiles = std::filesystem::remove_all(m_directoryPath);
		CATAPULT_LOG(debug)
				<< "deleted directory " << m_directoryPath << " and removed " << numRemovedFiles
				<< " files (exists? " << exists() << ")";

		// delete any empty parent directories too
		DeleteDirectoryIfEmpty(TempDirectoryGuard::DefaultName());
		DeleteDirectoryIfEmpty(Temp_Directory_Root);
	}

	TempDirectoryGuard::TempDirectoryGuard(const std::filesystem::path& directoryPath, bool) : m_directoryPath(directoryPath) {
		if (!StartsWithTempDirectoryRoot(directoryPath.generic_string()))
			CATAPULT_THROW_INVALID_ARGUMENT_1("temp directory does not start with temp directory root", directoryPath.generic_string());

		if (exists())
			CATAPULT_LOG(warning) << "creating directory " << m_directoryPath << " that already exists!";
		else
			CATAPULT_LOG(debug) << "creating directory " << m_directoryPath;

		std::filesystem::create_directories(m_directoryPath);
	}

	std::string TempDirectoryGuard::name() const {
		return m_directoryPath.generic_string();
	}

	bool TempDirectoryGuard::exists() const {
		return std::filesystem::exists(m_directoryPath);
	}

	std::string TempDirectoryGuard::DefaultName() {
		auto programLocation = std::filesystem::path(boost::dll::program_location().filename().generic_string());
		return (std::filesystem::path(Temp_Directory_Root) / programLocation).generic_string();
	}

	// endregion

	// region file guard

	TempFileGuard::TempFileGuard(const std::string& name) : m_name(name)
	{}

	std::string TempFileGuard::name() const {
		return (std::filesystem::path(m_directoryGuard.name()) / m_name).generic_string();
	}

	// endregion

	// region GetExplicitPluginsDirectory

	namespace {
		bool TryFindPluginsDirectory(const std::string& directory, bool recurse, std::string& pluginsDirectory) {
			if (!std::filesystem::is_directory(directory))
				return false;

			using std::filesystem::directory_iterator;
			for (auto iter = directory_iterator(directory); directory_iterator() != iter; ++iter) {
				if (std::string::npos != iter->path().generic_string().find("catapult.plugins")) {
					pluginsDirectory = directory;
					return true;
				}

				if (!recurse || !iter->is_directory())
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

	// region CountFilesAndDirectories

	size_t CountFilesAndDirectories(const std::filesystem::path& directoryPath) {
		auto begin = std::filesystem::directory_iterator(directoryPath);
		auto end = std::filesystem::directory_iterator();
		return static_cast<size_t>(std::distance(begin, end));
	}

	// endregion
}}

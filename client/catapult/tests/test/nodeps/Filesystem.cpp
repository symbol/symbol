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

namespace catapult { namespace test {

	// region directory guard

	TempDirectoryGuard::TempDirectoryGuard(const std::string& directoryName) : m_directoryPath(directoryName) {
		if (exists())
			CATAPULT_LOG(warning) << "creating directory " << m_directoryPath << " that already exists!";
		else
			CATAPULT_LOG(debug) << "creating directory " << m_directoryPath;

		boost::filesystem::create_directory(m_directoryPath);
	}

	TempDirectoryGuard::~TempDirectoryGuard() {
		auto numRemovedFiles = boost::filesystem::remove_all(m_directoryPath);
		CATAPULT_LOG(debug) << "deleted directory " << m_directoryPath << " and removed " << numRemovedFiles
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
				if (std::string::npos != iter->path().string().find("catapult.plugins")) {
					pluginsDirectory = directory;
					return true;
				}

				if (!recurse || !boost::filesystem::is_directory(iter->path()))
					continue;

				if (TryFindPluginsDirectory(iter->path().string(), false, pluginsDirectory))
					return true;
			}

			return false;
		}
	}

	std::string GetExplicitPluginsDirectory() {
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

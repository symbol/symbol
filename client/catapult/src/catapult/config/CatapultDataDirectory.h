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
#include "catapult/constants.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <iomanip>
#include <sstream>

namespace catapult { namespace config {

	// region CatapultDirectory

	/// Catapult directory.
	class CatapultDirectory {
	public:
		/// Creates a directory around \a directory.
		explicit CatapultDirectory(const boost::filesystem::path& directory) : m_directory(directory)
		{}

	public:
		/// Gets the directory path (as string).
		std::string str() const {
			return m_directory.generic_string();
		}

		/// Gets the directory path (as filesystem::path).
		const boost::filesystem::path& path() const {
			return m_directory;
		}

		/// Gets the path for the file with \a name.
		std::string file(const std::string& name) const {
			return (m_directory / name).generic_string();
		}

	public:
		/// Returns \c true if directory exits.
		bool exists() const;

		/// Creates this directory in a filesystem.
		void create() const;

		/// Creates all directories on the path to this directory in a filesystem.
		void createAll() const;

	private:
		boost::filesystem::path m_directory;
	};

	// endregion

	// region CatapultStorageDirectory

	/// Catapult storage directory.
	class CatapultStorageDirectory {
	public:
		/// Creates a storage directory around \a directory and \a identifier
		template<typename TIdentifier>
		CatapultStorageDirectory(const boost::filesystem::path& directory, TIdentifier identifier)
				: m_directory(directory)
				, m_identifier(identifier.unwrap())
		{}

	public:
		/// Gets the directory path (as string).
		std::string str() const {
			return m_directory.str();
		}

		/// Gets the path for a storage file with \a extension.
		std::string storageFile(const std::string& extension) const {
			std::ostringstream buffer;
			buffer << std::setfill('0') << std::setw(5) << (m_identifier % Files_Per_Storage_Directory);
			buffer << extension;

			return m_directory.file(buffer.str());
		}

		/// Gets the path for an index file built around \a prefix and \a extension.
		std::string indexFile(const std::string& prefix, const std::string& extension) const {
			return m_directory.file(prefix + extension);
		}

	private:
		CatapultDirectory m_directory;
		uint64_t m_identifier;
	};

	// endregion

	// region CatapultDataDirectory

	/// Catapult data directory.
	class CatapultDataDirectory {
	public:
		/// Creates a data directory around \a directory.
		explicit CatapultDataDirectory(const boost::filesystem::path& directory) : m_directory(directory)
		{}

	public:
		/// Gets the root data directory.
		CatapultDirectory rootDir() const {
			return CatapultDirectory(m_directory);
		}

		/// Gets the directory with \a name.
		CatapultDirectory dir(const std::string& name) const {
			return CatapultDirectory(m_directory / name);
		}

		/// Gets the spooling directory with \a name.
		CatapultDirectory spoolDir(const std::string& name) const {
			return CatapultDirectory(m_directory / "spool" / name);
		}

		/// Gets the storage directory built around \a identifier.
		template<typename TIdentifier>
		CatapultStorageDirectory storageDir(TIdentifier identifier) {
			std::ostringstream buffer;
			buffer << std::setfill('0') << std::setw(5) << (identifier.unwrap() / Files_Per_Storage_Directory);
			auto subDirectory = m_directory / buffer.str();
			return CatapultStorageDirectory(subDirectory, identifier);
		}

	private:
		boost::filesystem::path m_directory;
	};

	// endregion

	// region CatapultDataDirectoryPreparer

	/// Catapult data directory factory that automatically creates subdirectories.
	class CatapultDataDirectoryPreparer {
	public:
		/// Creates a data directory around \a directory.
		static CatapultDataDirectory Prepare(const boost::filesystem::path& directory) {
			CatapultDirectory(directory / "importance").create();
			CatapultDirectory(directory / "spool").create();
			return CatapultDataDirectory(directory);
		}
	};

	// endregion

	// region CatapultStorageDirectoryPreparer

	/// Catapult storage directory factory that creates subdirectories.
	class CatapultStorageDirectoryPreparer {
	public:
		/// Creates a storage directory around \a directory and \a identifier.
		template<typename TIdentifier>
		static CatapultStorageDirectory Prepare(const boost::filesystem::path& directory, TIdentifier identifier) {
			CatapultDataDirectory dataDirectory(directory);
			auto storageDirectory = dataDirectory.storageDir(identifier);
			CatapultDirectory(storageDirectory.str()).create();
			return storageDirectory;
		}
	};

	// endregion
}}

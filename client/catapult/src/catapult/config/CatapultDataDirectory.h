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
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>

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

	private:
		boost::filesystem::path m_directory;
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
			CreateDirectory(directory / "spool");
			return CatapultDataDirectory(directory);
		}

	private:
		static void CreateDirectory(const boost::filesystem::path& directory) {
			if (!boost::filesystem::exists(directory))
				boost::filesystem::create_directory(directory);
		}
	};

	// endregion
}}

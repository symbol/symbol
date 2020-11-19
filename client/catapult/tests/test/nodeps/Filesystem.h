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
#include <filesystem>
#include <string>

namespace catapult { namespace test {

	// region TempDirectoryGuard

	/// Uses RAII to delete a test directory.
	class TempDirectoryGuard final {
	public:
		/// Guards a default test directory.
		TempDirectoryGuard();

		/// Guards the directory with the specified name (\a directoryName).
		explicit TempDirectoryGuard(const std::string& directoryName);

		/// Deletes the guarded directory.
		~TempDirectoryGuard();

	private:
		// add a second parameter to disambiguate construction around string literal
		TempDirectoryGuard(const std::filesystem::path& directoryPath, bool);

	public:
		/// Gets the name of the guarded directory.
		std::string name() const;

	private:
		bool exists() const;

	public:
		/// Gets the default temp directory name.
		static std::string DefaultName();

	private:
		std::filesystem::path m_directoryPath;
	};

	// endregion

	// region TempFileGuard

	/// Uses RAII to delete a test file.
	class TempFileGuard {
	public:
		/// Guards the file with the specified \a name.
		explicit TempFileGuard(const std::string& name);

	public:
		/// Gets the name of the guarded file.
		std::string name() const;

	private:
		std::string m_name;
		TempDirectoryGuard m_directoryGuard;
	};

	// endregion

	/// Gets the explicit directory for a plugin test.
	std::string GetExplicitPluginsDirectory();

	/// Counts the number of files and directories in \a directoryPath.
	size_t CountFilesAndDirectories(const std::filesystem::path& directoryPath);
}}

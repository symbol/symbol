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
#include <string>

namespace catapult { namespace test {

	// region TempDirectoryGuard

	/// Uses RAII to delete a test directory.
	class TempDirectoryGuard final {
	public:
		/// Guards the directory with the specified name (\a directoryName).
		explicit TempDirectoryGuard(const std::string& directoryName = "../temp.dir");

		/// Deletes the guarded directory.
		~TempDirectoryGuard();

	public:
		/// Gets the name of the guarded directory.
		std::string name() const;

	private:
		bool exists() const;

	private:
		boost::filesystem::path m_directoryPath;
	};

	// endregion

	// region TempFileGuard

	/// Uses RAII to delete a test file.
	class TempFileGuard {
	public:
		/// Guards the file with the specified \a name.
		explicit TempFileGuard(const std::string& name);

		/// Deletes the guarded file.
		~TempFileGuard();

	public:
		/// Gets the name of the guarded file.
		const std::string& name() const;

	private:
		std::string m_name;
	};

	// endregion

	/// Gets the explicit directory for a plugin test.
	std::string GetExplicitPluginsDirectory();
}}

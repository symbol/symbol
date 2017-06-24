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

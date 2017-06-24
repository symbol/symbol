#pragma once
#include <string>

namespace catapult { namespace test {

	/// Prepares the storage by copying seed data into the \a destination directory.
	void PrepareStorage(const std::string& destination);

	/// Fakes file-based chain located at \a destination to \a height
	/// by setting proper value in index.dat and filling 00000/hashes.dat.
	void FakeHeight(const std::string& destination, uint64_t height);
}}

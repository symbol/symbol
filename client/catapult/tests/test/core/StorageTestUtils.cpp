#include "StorageTestUtils.h"
#include "catapult/io/RawFile.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace test {

	void PrepareStorage(const std::string& destination) {
		const std::string nemesisDirectory = "/00000";
		const std::string nemesisFilename = nemesisDirectory + "/00001.dat";
		boost::filesystem::create_directories(destination + nemesisDirectory);
		boost::filesystem::copy_file("../seed/mijin-test" + nemesisFilename, destination + nemesisFilename);
		const std::string nemesisHashFilename = nemesisDirectory + "/hashes.dat";
		boost::filesystem::copy_file("../seed/mijin-test" + nemesisHashFilename, destination + nemesisHashFilename);
	}

	void FakeHeight(const std::string& destination, uint64_t height) {
		const std::string nemesisDirectory = "/00000";
		const std::string nemesisHashFilename = destination + nemesisDirectory + "/hashes.dat";

		std::vector<uint8_t> data(height * Hash256_Size);
		{
			io::RawFile file(nemesisHashFilename, io::OpenMode::Read_Write);
			file.write(data);
		}

		--height;
		{
			io::RawFile file(destination + "/index.dat", io::OpenMode::Read_Write);
			file.write({ reinterpret_cast<const uint8_t*>(&height), sizeof(uint64_t) });
		}
	}
}}

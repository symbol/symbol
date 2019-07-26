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

#include "StorageTestUtils.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/RawFile.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace test {

	namespace {
		void SetIndexHeight(const std::string& destination, uint64_t height) {
			io::RawFile indexFile(destination + "/index.dat", io::OpenMode::Read_Write);
			io::Write64(indexFile, height);
		}
	}

	void PrepareStorage(const std::string& destination) {
#ifdef SIGNATURE_SCHEME_NIS1
		constexpr auto Source_Directory = "../seed/mijin-test.nis1";
#else
		constexpr auto Source_Directory = "../seed/mijin-test";
#endif

		const std::string nemesisDirectory = "/00000";
		const std::string nemesisFilename = nemesisDirectory + "/00001.dat";
		boost::filesystem::create_directories(destination + nemesisDirectory);
		boost::filesystem::copy_file(Source_Directory + nemesisFilename, destination + nemesisFilename);
		const std::string nemesisHashFilename = nemesisDirectory + "/hashes.dat";
		boost::filesystem::copy_file(Source_Directory + nemesisHashFilename, destination + nemesisHashFilename);

		SetIndexHeight(destination, 1);
	}

	void FakeHeight(const std::string& destination, uint64_t height) {
		const std::string nemesisDirectory = "/00000";
		const std::string nemesisHashFilename = destination + nemesisDirectory + "/hashes.dat";

		std::vector<uint8_t> hashesBuffer(height * Hash256::Size);
		{
			io::RawFile file(nemesisHashFilename, io::OpenMode::Read_Write);
			file.write(hashesBuffer);
		}

		SetIndexHeight(destination, --height);
	}
}}

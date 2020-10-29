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
#include "sdk/src/extensions/BlockExtensions.h"
#include "catapult/io/FileBlockStorage.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/RawFile.h"
#include "catapult/model/EntityHasher.h"
#include "tests/test/nodeps/Nemesis.h"
#include "tests/test/nodeps/TestNetworkConstants.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace test {

	namespace {
		constexpr auto Source_Directory = "../seed/private-test";

		void SetIndexHeight(const std::string& destination, uint64_t height) {
			io::RawFile indexFile(destination + "/index.dat", io::OpenMode::Read_Write);
			io::Write64(indexFile, height);
		}
	}

	void PrepareStorage(const std::string& destination) {
		PrepareStorageWithoutNemesis(destination);

		for (auto filename : { "proof.index.dat" })
			boost::filesystem::copy_file(std::string(Source_Directory) + "/" + filename, destination + "/" + filename);

		for (auto filename : { "00001.dat", "00001.proof", "hashes.dat", "proof.heights.dat" })
			boost::filesystem::copy_file(std::string(Source_Directory) + "/00000/" + filename, destination + "/00000/" + filename);
	}

	void PrepareStorageWithoutNemesis(const std::string& destination) {
		const std::string nemesisDirectory = "/00000";
		boost::filesystem::create_directories(destination + nemesisDirectory);

		SetIndexHeight(destination, 1);
	}

	void ModifyNemesis(const std::string& destination, const consumer<model::Block&, const model::BlockElement&>& modify) {
		// load from file storage to allow successive modifications
		io::FileBlockStorage storage(destination);
		auto pNemesisBlockElement = storage.loadBlockElement(Height(1));

		// modify nemesis block and resign it
		auto& nemesisBlock = const_cast<model::Block&>(pNemesisBlockElement->Block);
		modify(nemesisBlock, *pNemesisBlockElement);
		extensions::BlockExtensions(GetNemesisGenerationHashSeed()).signFullBlock(
				crypto::KeyPair::FromString(Test_Network_Nemesis_Private_Key),
				nemesisBlock);

		// overwrite the nemesis file in destination
		// (only the block and entity hash need to be rewritten; this works because block size does not change)
		io::RawFile nemesisFile(destination + "/00000/00001.dat", io::OpenMode::Read_Append);
		nemesisFile.write({ reinterpret_cast<const uint8_t*>(&nemesisBlock), nemesisBlock.Size });
		nemesisFile.write(model::CalculateHash(nemesisBlock));
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

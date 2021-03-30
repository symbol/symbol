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

#include "StorageTestUtils.h"
#include "sdk/src/extensions/BlockExtensions.h"
#include "catapult/io/FileBlockStorage.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/RawFile.h"
#include "catapult/model/EntityHasher.h"
#include "tests/test/nodeps/Nemesis.h"
#include "tests/test/nodeps/TestConstants.h"
#include "tests/test/nodeps/TestNetworkConstants.h"
#include <filesystem>

namespace catapult { namespace test {

	namespace {
		constexpr auto Source_Directory = "../seed/private-test";

		void MakeReadOnly(const std::filesystem::path& filepath) {
			std::filesystem::permissions(filepath, std::filesystem::perms::owner_read, std::filesystem::perm_options::replace);
		}

		void SetIndexHeight(const std::string& destination, uint64_t height) {
			io::RawFile indexFile(destination + "/index.dat", io::OpenMode::Read_Write);
			io::Write64(indexFile, height);
		}

		void PrepareStorage(const std::string& destination, uint32_t fileDatabaseBatchSize) {
			PrepareStorageWithoutNemesis(destination);

			for (auto filename : { "proof.index.dat" })
				std::filesystem::copy_file(std::string(Source_Directory) + "/" + filename, destination + "/" + filename);

			for (auto filename : { "hashes.dat", "proof.heights.dat" })
				std::filesystem::copy_file(std::string(Source_Directory) + "/00000/" + filename, destination + "/00000/" + filename);

			for (auto extension : { ".dat", ".proof" }) {
				if (1 == fileDatabaseBatchSize) {
					std::filesystem::copy_file(
							std::string(Source_Directory) + "/00000/00001" + extension,
							destination + "/00000/00001" + extension);
					continue;
				}

				io::RawFile inputFile(std::string(Source_Directory) + "/00000/00001" + extension, io::OpenMode::Read_Only);
				io::RawFile outputFile(destination + "/00000/00000" + extension, io::OpenMode::Read_Write);

				// write file database header
				auto headerSize = fileDatabaseBatchSize * sizeof(uint64_t);
				outputFile.write(std::vector<uint8_t>(headerSize));
				outputFile.seek(sizeof(uint64_t));
				Write64(outputFile, headerSize);
				outputFile.seek(headerSize);

				// copy input file contents
				std::vector<uint8_t> inputBuffer(inputFile.size());
				inputFile.read(inputBuffer);
				outputFile.write(inputBuffer);
			}
		}
	}

	void PrepareStorage(const std::string& destination) {
		PrepareStorage(destination, File_Database_Batch_Size);
	}

	void PrepareSeedStorage(const std::string& destination) {
		PrepareStorage(destination, 1);

		for (const auto& entry : std::filesystem::recursive_directory_iterator(destination)) {
			if (!entry.is_regular_file())
				continue;

			MakeReadOnly(entry.path());
		}
	}

	void PrepareStorageWithoutNemesis(const std::string& destination) {
		const std::string nemesisDirectory = "/00000";
		std::filesystem::create_directories(destination + nemesisDirectory);

		SetIndexHeight(destination, 1);
	}

	namespace {
		std::string GetNemesisFilename(const std::string& destination, uint32_t fileDatabaseBatchSize) {
			return destination + "/00000/0000" + (1 == fileDatabaseBatchSize ? "1.dat" : "0.dat");
		}

		void ModifyNemesis(
				const std::string& destination,
				uint32_t fileDatabaseBatchSize,
				const consumer<model::Block&, const model::BlockElement&>& modify) {
			// load from file storage to allow successive modifications
			io::FileBlockStorage storage(destination, fileDatabaseBatchSize);
			auto pNemesisBlockElement = storage.loadBlockElement(Height(1));

			// modify nemesis block and resign it
			auto& nemesisBlock = const_cast<model::Block&>(pNemesisBlockElement->Block);
			modify(nemesisBlock, *pNemesisBlockElement);
			extensions::BlockExtensions(GetNemesisGenerationHashSeed()).signFullBlock(
					crypto::KeyPair::FromString(Test_Network_Nemesis_Private_Key),
					nemesisBlock);

			// overwrite the nemesis file in destination
			// (only the block and entity hash need to be rewritten; this works because block size does not change)
			auto nemesisFilename = GetNemesisFilename(destination, fileDatabaseBatchSize);
			io::RawFile nemesisFile(nemesisFilename, io::OpenMode::Read_Append);
			if (1 != fileDatabaseBatchSize)
				nemesisFile.seek(fileDatabaseBatchSize * sizeof(uint64_t));

			nemesisFile.write({ reinterpret_cast<const uint8_t*>(&nemesisBlock), nemesisBlock.Size });
			nemesisFile.write(model::CalculateHash(nemesisBlock));
		}
	}

	void ModifyNemesis(const std::string& destination, const consumer<model::Block&, const model::BlockElement&>& modify) {
		ModifyNemesis(destination, File_Database_Batch_Size, modify);
	}

	void ModifySeedNemesis(const std::string& destination, const consumer<model::Block&, const model::BlockElement&>& modify) {
		auto nemesisFilename = GetNemesisFilename(destination, 1);
		std::filesystem::permissions(nemesisFilename, std::filesystem::perms::owner_write, std::filesystem::perm_options::add);

		ModifyNemesis(destination, 1, modify);

		MakeReadOnly(nemesisFilename);
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

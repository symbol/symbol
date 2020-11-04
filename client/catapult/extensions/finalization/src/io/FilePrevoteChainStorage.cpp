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

#include "FilePrevoteChainStorage.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/io/FilesystemUtils.h"
#include "catapult/io/IndexFile.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/model/Elements.h"
#include "catapult/preprocessor.h"

namespace catapult { namespace io {

	namespace {
		static constexpr auto Block_File_Extension = ".dat";

		auto GetStorageDirectory(const std::string& dataDirectory, Height height) {
			return config::CatapultStorageDirectoryPreparer::Prepare(dataDirectory, height);
		}

		std::string GetBlockPath(const std::string& dataDirectory, Height height) {
			auto storageDirectory = GetStorageDirectory(dataDirectory, height);
			return storageDirectory.storageFile(Block_File_Extension);
		}

		config::CatapultDirectory GetRoundDirectory(const std::string& dataDirectory, const model::FinalizationRound& round) {
			auto votingDirectory = config::CatapultDataDirectory(dataDirectory).dir("voting");

			std::ostringstream roundName;
			roundName << round.Epoch << "_" << round.Point;
			return config::CatapultDataDirectory(votingDirectory.path()).dir(roundName.str());
		}

		std::string GetVotingBlockPath(const config::CatapultDirectory& roundDirectory, Height height) {
			std::ostringstream buffer;
			buffer << height << Block_File_Extension;

			return roundDirectory.file(buffer.str());
		}

		void CopyBlockFile(const boost::filesystem::path& source, const boost::filesystem::path& destination) {
			if (boost::filesystem::exists(destination))
				boost::filesystem::remove(destination);

			boost::filesystem::copy_file(source, destination);
		}

		void CopyChain(
				const std::string& dataDirectory,
				const config::CatapultDirectory& roundDirectory,
				Height startHeight,
				size_t numBlocks) {
			for (auto i = 0u; i < numBlocks; ++i) {
				auto sourcePath = GetBlockPath(dataDirectory, startHeight + Height(i));
				auto destinationPath = GetVotingBlockPath(roundDirectory, startHeight + Height(i));
				CopyBlockFile(sourcePath, destinationPath);
			}

			auto indexPath = roundDirectory.file("index.dat");
			IndexFile index(indexPath);
			index.set(startHeight.unwrap());
		}

		std::unique_ptr<model::Block> LoadBlock(const std::string& pathname) {
			RawFile blockFile(pathname, OpenMode::Read_Only);
			auto size = Read32(blockFile);
			blockFile.seek(0);

			auto pBlock = utils::MakeUniqueWithSize<model::Block>(size);
			blockFile.read({ reinterpret_cast<uint8_t*>(pBlock.get()), size });
			return pBlock;
		}

		auto GetBlockHash(const std::string& pathname) {
			RawFile blockFile(pathname, OpenMode::Read_Only);
			auto size = Read32(blockFile);

			// skip block data
			blockFile.seek(size);

			decltype(model::BlockElement::EntityHash) hash;
			blockFile.read(hash);
			return hash;
		}
	}

	FilePrevoteChainStorage::FilePrevoteChainStorage(const std::string& dataDirectory) : m_dataDirectory(dataDirectory)
	{}

	bool FilePrevoteChainStorage::contains(const model::FinalizationRound& round, const model::HeightHashPair& heightHashPair) const {
		auto roundDirectory = GetRoundDirectory(m_dataDirectory, round);
		if (!roundDirectory.exists())
			return false;

		auto blockPath = GetVotingBlockPath(roundDirectory, heightHashPair.Height);
		if (!boost::filesystem::exists(blockPath))
			return false;

		return GetBlockHash(blockPath) == heightHashPair.Hash;
	}

	model::BlockRange FilePrevoteChainStorage::load(const model::FinalizationRound& round, Height maxHeight) const {
		auto roundDirectory = GetRoundDirectory(m_dataDirectory, round);
		auto indexPath = roundDirectory.file("index.dat");
		IndexFile index(indexPath);
		if (!index.exists())
			CATAPULT_THROW_INVALID_ARGUMENT_1("round does not exist", round);

		auto startHeight = Height(index.get());

		std::vector<model::BlockRange> chain;
		auto i = 0u;
		while (true) {
			if (startHeight + Height(i) > maxHeight)
				break;

			auto blockPath = GetVotingBlockPath(roundDirectory, startHeight + Height(i));
			if (!boost::filesystem::exists(blockPath))
				break;

			chain.push_back(model::BlockRange::FromEntity(LoadBlock(blockPath)));
			++i;
		}

		return model::BlockRange::MergeRanges(std::move(chain));
	}

	void FilePrevoteChainStorage::save(const BlockStorageView&, const PrevoteChainDescriptor& descriptor) {
		// BlockStorageView holds lock on block storage
		remove(descriptor.Round);

		auto roundDirectory = GetRoundDirectory(m_dataDirectory, descriptor.Round);
		roundDirectory.createAll();
		CopyChain(m_dataDirectory, roundDirectory, descriptor.Height, descriptor.HashesCount);
	}

	void FilePrevoteChainStorage::remove(const model::FinalizationRound& round) {
		auto roundDirectory = GetRoundDirectory(m_dataDirectory, round);
		if (!roundDirectory.exists())
			return;

		PurgeDirectory(roundDirectory.str());
		boost::filesystem::remove(roundDirectory.path());
	}
}}

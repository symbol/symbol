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

#include "NemesisStateHasher.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/model/Elements.h"
#include "catapult/utils/HexFormatter.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace tools { namespace nemgen {

	namespace {
		class TempDirectoryGuard {
		public:
			explicit TempDirectoryGuard(const std::string& directoryPath) : m_directoryPath(directoryPath)
			{}

			~TempDirectoryGuard() {
				auto numRemovedFiles = boost::filesystem::remove_all(m_directoryPath);
				CATAPULT_LOG(info)
						<< "deleted directory " << m_directoryPath << " and removed " << numRemovedFiles
						<< " files (exists? " << boost::filesystem::exists(m_directoryPath) << ")";
			}

		private:
			std::string m_directoryPath;
		};

		std::string Format(const BlockStateHashInfo& blockStateHashInfo) {
			std::ostringstream out;
			out
					<< "State Hash: " << utils::HexFormat(blockStateHashInfo.StateHash) << std::endl
					<< "--- Components (" << blockStateHashInfo.SubCacheMerkleRoots.size() << ") ---" << std::endl;

			for (const auto& subCacheMerkleRoot : blockStateHashInfo.SubCacheMerkleRoots)
				out << " + " << utils::HexFormat(subCacheMerkleRoot) << std::endl;

			return out.str();
		}
	}

	NemesisStateHashDescriptor CalculateAndLogNemesisStateHash(
			const model::BlockElement& blockElement,
			const config::LocalNodeConfiguration& config,
			CacheDatabaseCleanupMode databaseCleanupMode) {
		if (!config.Node.ShouldUseCacheDatabaseStorage || !config.BlockChain.ShouldEnableVerifiableState)
			CATAPULT_LOG(warning) << "cache database storage and verifiable state must both be enabled to calculate state hash";

		// clean up the rocks database after each execution
		std::unique_ptr<TempDirectoryGuard> pDatabaseGuard;
		if (CacheDatabaseCleanupMode::Purge == databaseCleanupMode) {
			auto cacheDatabaseDirectory = (boost::filesystem::path(config.User.DataDirectory) / "statedb").generic_string();
			pDatabaseGuard = std::make_unique<TempDirectoryGuard>(cacheDatabaseDirectory);
		}

		CATAPULT_LOG(info) << "calculating nemesis state hash";
		auto blockStateHashInfo = CalculateNemesisStateHash(blockElement, config);
		std::ostringstream out;
		out
				<< "Height: " << blockElement.Block.Height << std::endl
				<< "Generation Hash: " << utils::HexFormat(blockElement.GenerationHash) << std::endl
				<< "Hash: " << utils::HexFormat(blockElement.EntityHash) << std::endl
				<< Format(blockStateHashInfo);

		return { blockStateHashInfo.StateHash, out.str() };
	}
}}}

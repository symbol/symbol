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

#include "tools/ToolMain.h"
#include "BlockGenerator.h"
#include "BlockSaver.h"
#include "NemesisConfigurationLoader.h"
#include "NemesisExecutionHasher.h"
#include "tools/ToolConfigurationUtils.h"
#include "catapult/io/RawFile.h"

namespace catapult { namespace tools { namespace nemgen {

	namespace {
		void WriteToFile(const std::string& filePath, const std::string& content) {
			io::RawFile file(filePath, io::OpenMode::Read_Write);
			file.write({ reinterpret_cast<const uint8_t*>(content.data()), content.size() });
		}

		class NemGenTool : public Tool {
		public:
			std::string name() const override {
				return "Nemesis Block Generator Tool";
			}

			void prepareOptions(OptionsBuilder& optionsBuilder, OptionsPositional&) override {
				optionsBuilder("resources,r",
						OptionsValue<std::string>(m_resourcesPath)->default_value(".."),
						"the path to the resources directory");

				optionsBuilder("nemesisProperties,p",
						OptionsValue<std::string>(m_nemesisPropertiesFilePath),
						"the path to the nemesis properties file");

				optionsBuilder("summary,s",
						OptionsValue<std::string>(m_summaryFilePath),
						"the path to summary output file (default: <bindir>/summary.txt)");

				optionsBuilder("no-summary,n",
						OptionsSwitch(),
						"don't generate summary file");

				optionsBuilder("useTemporaryCacheDatabase,t",
						OptionsSwitch(),
						"true if a temporary cache database should be created and destroyed");
			}

			int run(const Options& options) override {
				// 1. load config
				auto config = LoadConfiguration(m_resourcesPath);
				auto nemesisConfig = LoadNemesisConfiguration(m_nemesisPropertiesFilePath);
				if (!LogAndValidateNemesisConfiguration(nemesisConfig))
					return -1;

				// 2. create the nemesis block element
				auto databaseCleanupMode = options["useTemporaryCacheDatabase"].as<bool>()
						? CacheDatabaseCleanupMode::Purge
						: CacheDatabaseCleanupMode::None;
				auto pBlock = CreateNemesisBlock(nemesisConfig);
				auto blockElement = CreateNemesisBlockElement(nemesisConfig, *pBlock);
				auto executionHashesDescriptor = CalculateAndLogNemesisExecutionHashes(blockElement, config, databaseCleanupMode);
				if (!options["no-summary"].as<bool>()) {
					if (m_summaryFilePath.empty())
						m_summaryFilePath = nemesisConfig.BinDirectory + "/summary.txt";

					WriteToFile(m_summaryFilePath, executionHashesDescriptor.Summary);
				}

				// 3. update block with result of execution
				CATAPULT_LOG(info) << "*** Nemesis Summary ***" << std::endl << executionHashesDescriptor.Summary;
				blockElement.EntityHash = UpdateNemesisBlock(nemesisConfig, *pBlock, executionHashesDescriptor);
				blockElement.SubCacheMerkleRoots = executionHashesDescriptor.SubCacheMerkleRoots;
				if (config.BlockChain.EnableVerifiableReceipts)
					blockElement.OptionalStatement = std::move(executionHashesDescriptor.pBlockStatement);

				// 4. save the nemesis block element
				SaveNemesisBlockElement(blockElement, nemesisConfig);
				return 0;
			}

		private:
			std::string m_resourcesPath;
			std::string m_nemesisPropertiesFilePath;
			std::string m_summaryFilePath;
		};
	}
}}}

int main(int argc, const char** argv) {
	catapult::tools::nemgen::NemGenTool nemGenTool;
	return catapult::tools::ToolMain(argc, argv, nemGenTool);
}

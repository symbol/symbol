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
#include "SslClient.h"
#include "tools/ToolConfigurationUtils.h"
#include "catapult/config/ConfigurationFileLoader.h"
#include "catapult/ionet/Node.h"
#include "catapult/thread/IoThreadPool.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace tools { namespace ssl {

	namespace {
		auto GetHarvestingKeyPair(const boost::filesystem::path& resourcesPath) {
			auto bag = utils::ConfigurationBag::FromPath((resourcesPath / "resources" / "config-harvesting.properties").generic_string());
			std::string harvesterSigningPrivateKey;
			utils::LoadIniProperty(bag, "harvesting", "HarvesterSigningPrivateKey", harvesterSigningPrivateKey);
			return crypto::KeyPair::FromString(harvesterSigningPrivateKey);
		}

		class SslTool : public Tool {
		public:
			std::string name() const override {
				return "SSL Test Tool";
			}

			void prepareOptions(OptionsBuilder& optionsBuilder, OptionsPositional&) override {
				optionsBuilder("resources,r",
						OptionsValue<std::string>()->default_value(".."),
						"the path to the resources directory");

				optionsBuilder(
						"scenario",
						OptionsValue<uint16_t>()->default_value(0),
						"test scenario id\n"
						" 0 - valid certificate chain,\n"
						" 1 - malformed node certificate signature,\n"
						" 2 - malformed CA certificate signature,\n"
						" 3 - single self-signed CA certificate,\n"
						" 4 - two-level certificate chain with same key,\n"
						" 5 - three-level certificate chain,\n"
						" 6 - expired node certificate,\n"
						" 7 - expired CA certificate");
				optionsBuilder(
						"tempCertificateDirectory,t",
						OptionsValue<std::string>(m_tempCertificateDirectory),
						"directory with generated certificate files (will be wiped)");

				optionsBuilder("host,s", OptionsValue<std::string>(m_host)->default_value("127.0.0.1"), "ssl host");
				optionsBuilder("port,p", OptionsValue<uint16_t>(m_port)->default_value(7900), "ssl port");

				optionsBuilder("expected", OptionsValue<std::string>(m_expectedResult), "expected result (failure|success)");
			}

			int run(const Options& options) override {
				auto pPool = CreateStartedThreadPool(2);

				auto scenarioId = static_cast<ScenarioId>(options["scenario"].as<uint16_t>());
				if (scenarioId >= ScenarioId::Max_Value)
					CATAPULT_THROW_RUNTIME_ERROR_1("invalid value of scenario id", utils::to_underlying_type(scenarioId));

				// note: harvester key will be used to generate CA certificate
				auto keyPair = GetHarvestingKeyPair(options["resources"].as<std::string>());
				bool isSuccess = true;
				try {
					SslClient sslClient(*pPool, std::move(keyPair), m_tempCertificateDirectory, scenarioId);
					auto chainStatistics = sslClient.connect(ionet::NodeEndpoint{ m_host, m_port });

					CATAPULT_LOG(info) << " height: " << chainStatistics.Height;
					CATAPULT_LOG(info) << "  score: " << chainStatistics.Score;

					pPool->join();
				} catch (const std::exception& e) {
					CATAPULT_LOG(fatal) << "exception occured: " << e.what();

					isSuccess = false;
				}

				// 'not' because we want to return 0 on success
				auto expectedSuccess = "success" == m_expectedResult;
				return expectedSuccess != isSuccess;
			}

		private:
			std::string m_tempCertificateDirectory;
			std::string m_host;
			uint16_t m_port;
			std::string m_expectedResult;
		};
	}
}}}

int main(int argc, const char** argv) {
	catapult::tools::ssl::SslTool tool;
	return catapult::tools::ToolMain(argc, argv, tool);
}

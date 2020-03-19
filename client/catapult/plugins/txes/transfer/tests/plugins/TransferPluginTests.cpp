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

#include "src/plugins/TransferPlugin.h"
#include "plugins/txes/transfer/src/model/TransferEntityType.h"
#include "tests/test/net/CertificateLocator.h"
#include "tests/test/plugins/PluginManagerFactory.h"
#include "tests/test/plugins/PluginTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace plugins {

	namespace {
		template<bool EnableAutoDetection>
		struct TransferPluginTraits : public test::EmptyPluginTraits {
		public:
			template<typename TAction>
			static void RunTestAfterRegistration(TAction action) {
				// Arrange:
				auto config = model::BlockChainConfiguration::Uninitialized();
				config.Plugins.emplace("catapult.plugins.transfer", utils::ConfigurationBag({{ "", { { "maxMessageSize", "0" } } }}));

				auto userConfig = config::UserConfiguration::Uninitialized();
				userConfig.CertificateDirectory = test::GetDefaultCertificateDirectory();
				userConfig.EnableDelegatedHarvestersAutoDetection = EnableAutoDetection;

				auto manager = test::CreatePluginManager(config, userConfig);
				RegisterTransferSubsystem(manager);

				// Act:
				action(manager);
			}

		public:
			static std::vector<model::EntityType> GetTransactionTypes() {
				return { model::Entity_Type_Transfer };
			}

			static std::vector<std::string> GetStatelessValidatorNames() {
				return { "TransferMessageValidator", "TransferMosaicsValidator" };
			}
		};

		struct TransferPluginWithoutMessageProcessingTraits : public TransferPluginTraits<false> {};

		struct TransferPluginWithMessageProcessingTraits : public TransferPluginTraits<true> {
		public:
			static std::vector<std::string> GetObserverNames() {
				return { "TransferMessageObserver" };
			}

			static std::vector<std::string> GetPermanentObserverNames() {
				return GetObserverNames();
			}
		};
	}

	DEFINE_PLUGIN_TESTS(TransferPluginWithoutMessageProcessingTests, TransferPluginWithoutMessageProcessingTraits)
	DEFINE_PLUGIN_TESTS(TransferPluginWithMessageProcessingTests, TransferPluginWithMessageProcessingTraits)
}}

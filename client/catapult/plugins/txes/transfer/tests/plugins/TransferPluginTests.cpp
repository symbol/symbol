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
#include "tests/test/plugins/PluginTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace plugins {

	namespace {
		struct TransferPluginTraits {
		public:
			template<typename TAction>
			static void RunTestAfterRegistration(TAction action) {
				// Arrange:
				auto config = model::BlockChainConfiguration::Uninitialized();
				config.Plugins.emplace("catapult.plugins.transfer", utils::ConfigurationBag({{ "", { { "maxMessageSize", "0" } } }}));
				PluginManager manager(config, StorageConfiguration());
				RegisterTransferSubsystem(manager);

				// Act:
				action(manager);
			}

		public:
			static std::vector<model::EntityType> GetTransactionTypes() {
				return { model::Entity_Type_Transfer };
			}

			static std::vector<std::string> GetCacheNames() {
				return {};
			}

			static std::vector<ionet::PacketType> GetDiagnosticPacketTypes() {
				return {};
			}

			static std::vector<std::string> GetDiagnosticCounterNames() {
				return {};
			}

			static std::vector<std::string> GetStatelessValidatorNames() {
				return { "TransferMessageValidator", "TransferMosaicsValidator" };
			}

			static std::vector<std::string> GetStatefulValidatorNames() {
				return {};
			}

			static std::vector<std::string> GetObserverNames() {
				return {};
			}

			static std::vector<std::string> GetPermanentObserverNames() {
				return {};
			}
		};
	}

	DEFINE_PLUGIN_TESTS(TransferPluginTests, TransferPluginTraits)
}}

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

#include "src/plugins/MultisigPlugin.h"
#include "plugins/txes/multisig/src/model/MultisigEntityType.h"
#include "tests/test/plugins/PluginTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace plugins {

	namespace {
		struct MultisigPluginTraits {
		public:
			template<typename TAction>
			static void RunTestAfterRegistration(TAction action) {
				// Arrange:
				auto config = model::BlockChainConfiguration::Uninitialized();
				config.Plugins.emplace("catapult.plugins.multisig", utils::ConfigurationBag({{
					"",
					{
						{ "maxMultisigDepth", "0" },
						{ "maxCosignersPerAccount", "0" },
						{ "maxCosignedAccountsPerAccount", "0" }
					}
				}}));

				PluginManager manager(config, StorageConfiguration());
				RegisterMultisigSubsystem(manager);

				// Act:
				action(manager);
			}

		public:
			static std::vector<model::EntityType> GetTransactionTypes() {
				return { model::Entity_Type_Modify_Multisig_Account };
			}

			static std::vector<std::string> GetCacheNames() {
				return { "MultisigCache" };
			}

			static std::vector<ionet::PacketType> GetDiagnosticPacketTypes() {
				return { ionet::PacketType::Multisig_Infos, ionet::PacketType::Multisig_State_Path };
			}

			static std::vector<std::string> GetDiagnosticCounterNames() {
				return {};
			}

			static std::vector<std::string> GetStatelessValidatorNames() {
				return { "ModifyMultisigCosignersValidator" };
			}

			static std::vector<std::string> GetStatefulValidatorNames() {
				return {
					"MultisigPermittedOperationValidator",
					"ModifyMultisigMaxCosignedAccountsValidator",
					"ModifyMultisigMaxCosignersValidator",
					"ModifyMultisigInvalidCosignersValidator",
					"ModifyMultisigInvalidSettingsValidator",
					"ModifyMultisigLoopAndLevelValidator",
					"MultisigAggregateEligibleCosignersValidator",
					"MultisigAggregateSufficientCosignersValidator"
				};
			}

			static std::vector<std::string> GetObserverNames() {
				return { "ModifyMultisigCosignersObserver", "ModifyMultisigSettingsObserver" };
			}

			static std::vector<std::string> GetPermanentObserverNames() {
				return { "ModifyMultisigCosignersObserver", "ModifyMultisigSettingsObserver" };
			}
		};
	}

	DEFINE_PLUGIN_TESTS(MultisigPluginTests, MultisigPluginTraits)
}}

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

#include "src/plugins/MultisigPlugin.h"
#include "plugins/txes/multisig/src/model/MultisigEntityType.h"
#include "tests/test/plugins/PluginManagerFactory.h"
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
						{ "maxCosignatoriesPerAccount", "0" },
						{ "maxCosignedAccountsPerAccount", "0" }
					}
				}}));

				auto manager = test::CreatePluginManager(config);
				RegisterMultisigSubsystem(manager);

				// Act:
				action(manager);
			}

		public:
			static std::vector<model::EntityType> GetTransactionTypes() {
				return { model::Entity_Type_Multisig_Account_Modification };
			}

			static std::vector<std::string> GetCacheNames() {
				return { "MultisigCache" };
			}

			static std::vector<ionet::PacketType> GetNonDiagnosticPacketTypes() {
				return { ionet::PacketType::Multisig_State_Path };
			}

			static std::vector<ionet::PacketType> GetDiagnosticPacketTypes() {
				return { ionet::PacketType::Multisig_Infos };
			}

			static std::vector<std::string> GetDiagnosticCounterNames() {
				return { "MULTISIG C" };
			}

			static std::vector<std::string> GetStatelessValidatorNames() {
				return { "MultisigCosignatoriesValidator" };
			}

			static std::vector<std::string> GetStatefulValidatorNames() {
				return {
					"MultisigPermittedOperationValidator",
					"MultisigMaxCosignedAccountsValidator",
					"MultisigMaxCosignatoriesValidator",
					"MultisigInvalidCosignatoriesValidator",
					"MultisigInvalidSettingsValidator",
					"MultisigLoopAndLevelValidator",
					"MultisigAggregateEligibleCosignatoriesValidator",
					"MultisigAggregateSufficientCosignatoriesValidator"
				};
			}

			static std::vector<std::string> GetObserverNames() {
				return { "MultisigCosignatoriesObserver", "MultisigSettingsObserver" };
			}

			static std::vector<std::string> GetPermanentObserverNames() {
				return GetObserverNames();
			}
		};
	}

	DEFINE_PLUGIN_TESTS(MultisigPluginTests, MultisigPluginTraits)
}}

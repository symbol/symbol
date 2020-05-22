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

#include "src/plugins/AccountRestrictionPlugin.h"
#include "src/model/AccountRestrictionEntityType.h"
#include "tests/test/plugins/PluginManagerFactory.h"
#include "tests/test/plugins/PluginTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace plugins {

	namespace {
		struct AccountRestrictionPluginTraits {
		public:
			template<typename TAction>
			static void RunTestAfterRegistration(TAction action) {
				// Arrange:
				auto config = model::BlockChainConfiguration::Uninitialized();
				config.Plugins.emplace("catapult.plugins.restrictionaccount", utils::ConfigurationBag({{
					"",
					{
						{ "maxAccountRestrictionValues", "10" }
					}
				}}));

				auto manager = test::CreatePluginManager(config);
				RegisterAccountRestrictionSubsystem(manager);

				// Act:
				action(manager);
			}

		public:
			static std::vector<model::EntityType> GetTransactionTypes() {
				return {
					model::Entity_Type_Account_Address_Restriction,
					model::Entity_Type_Account_Mosaic_Restriction,
					model::Entity_Type_Account_Operation_Restriction
				};
			}

			static std::vector<std::string> GetCacheNames() {
				return { "AccountRestrictionCache" };
			}

			static std::vector<ionet::PacketType> GetNonDiagnosticPacketTypes() {
				return { ionet::PacketType::Account_Restrictions_State_Path };
			}

			static std::vector<ionet::PacketType> GetDiagnosticPacketTypes() {
				return { ionet::PacketType::Account_Restrictions_Infos };
			}

			static std::vector<std::string> GetDiagnosticCounterNames() {
				return { "ACCTREST C" };
			}

			static std::vector<std::string> GetStatelessValidatorNames() {
				return {
					"AccountRestrictionFlagsValidator",

					"AccountOperationRestrictionModificationValuesValidator"
				};
			}

			static std::vector<std::string> GetStatefulValidatorNames() {
				return {
					"AccountAddressRestrictionRedundantModificationValidator",
					"AccountAddressRestrictionValueModificationValidator",
					"MaxAccountAddressRestrictionValuesValidator",
					"AddressInteractionValidator",
					"AccountAddressRestrictionNoSelfModificationValidator",

					"AccountMosaicRestrictionRedundantModificationValidator",
					"AccountMosaicRestrictionValueModificationValidator",
					"MaxAccountMosaicRestrictionValuesValidator",
					"MosaicRecipientValidator",

					"AccountOperationRestrictionRedundantModificationValidator",
					"AccountOperationRestrictionValueModificationValidator",
					"MaxAccountOperationRestrictionValuesValidator",
					"OperationRestrictionValidator",
					"AccountOperationRestrictionNoSelfBlockingValidator"
				};
			}

			static std::vector<std::string> GetObserverNames() {
				return {
					"AccountAddressRestrictionValueModificationObserver",
					"AccountMosaicRestrictionValueModificationObserver",
					"AccountOperationRestrictionValueModificationObserver"
				};
			}

			static std::vector<std::string> GetPermanentObserverNames() {
				return GetObserverNames();
			}
		};
	}

	DEFINE_PLUGIN_TESTS(AccountRestrictionPluginTests, AccountRestrictionPluginTraits)
}}

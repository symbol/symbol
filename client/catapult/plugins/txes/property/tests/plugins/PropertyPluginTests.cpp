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

#include "src/plugins/PropertyPlugin.h"
#include "src/model/PropertyEntityType.h"
#include "tests/test/plugins/PluginTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace plugins {

	namespace {
		struct PropertyPluginTraits {
		public:
			template<typename TAction>
			static void RunTestAfterRegistration(TAction action) {
				// Arrange:
				auto config = model::BlockChainConfiguration::Uninitialized();
				config.Plugins.emplace("catapult.plugins.property", utils::ConfigurationBag({{
					"",
					{
						{ "maxPropertyValues", "10" },
					}
				}}));

				PluginManager manager(config, StorageConfiguration());
				RegisterPropertySubsystem(manager);

				// Act:
				action(manager);
			}

		public:
			static std::vector<model::EntityType> GetTransactionTypes() {
				return {
					model::Entity_Type_Address_Property,
					model::Entity_Type_Mosaic_Property,
					model::Entity_Type_Transaction_Type_Property
				};
			}

			static std::vector<std::string> GetCacheNames() {
				return { "PropertyCache" };
			}

			static std::vector<ionet::PacketType> GetNonDiagnosticPacketTypes() {
				return { ionet::PacketType::Account_Properties_State_Path };
			}

			static std::vector<ionet::PacketType> GetDiagnosticPacketTypes() {
				return { ionet::PacketType::Account_Properties_Infos };
			}

			static std::vector<std::string> GetDiagnosticCounterNames() {
				return { "PROPERTY C" };
			}

			static std::vector<std::string> GetStatelessValidatorNames() {
				return {
					"PropertyTypeValidator",
					"AddressPropertyModificationTypesValidator",
					"PropertyAddressNoSelfModificationValidator",
					"MosaicPropertyModificationTypesValidator",
					"TransactionTypePropertyModificationTypesValidator",
					"TransactionTypePropertyModificationValuesValidator"
				};
			}

			static std::vector<std::string> GetStatefulValidatorNames() {
				return {
					"AddressPropertyRedundantModificationValidator",
					"AddressPropertyValueModificationValidator",
					"MaxAddressPropertyValuesValidator",
					"AddressInteractionValidator",
					"MosaicPropertyRedundantModificationValidator",
					"MosaicPropertyValueModificationValidator",
					"MaxMosaicPropertyValuesValidator",
					"MosaicRecipientValidator",
					"TransactionTypePropertyRedundantModificationValidator",
					"TransactionTypePropertyValueModificationValidator",
					"MaxTransactionTypePropertyValuesValidator",
					"TransactionTypeValidator",
					"TransactionTypeNoSelfBlockingValidator"
				};
			}

			static std::vector<std::string> GetObserverNames() {
				return {
					"AddressPropertyValueModificationObserver",
					"MosaicPropertyValueModificationObserver",
					"TransactionTypePropertyValueModificationObserver"
				};
			}

			static std::vector<std::string> GetPermanentObserverNames() {
				return GetObserverNames();
			}
		};
	}

	DEFINE_PLUGIN_TESTS(PropertyPluginTests, PropertyPluginTraits)
}}

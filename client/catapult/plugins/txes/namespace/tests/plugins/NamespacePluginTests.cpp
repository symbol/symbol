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

#include "src/plugins/NamespacePlugin.h"
#include "src/model/MosaicEntityType.h"
#include "src/model/NamespaceEntityType.h"
#include "tests/test/plugins/PluginTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace plugins {

	namespace {
		struct NamespacePluginTraits {
		public:
			template<typename TAction>
			static void RunTestAfterRegistration(TAction action) {
				// Arrange:
				auto config = model::BlockChainConfiguration::Uninitialized();
				config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(1);
				config.BlockPruneInterval = 150;
				config.Plugins.emplace("catapult.plugins.namespace", utils::ConfigurationBag({{
					"",
					{
						{ "maxNameSize", "0" },
						{ "maxNamespaceDuration", "0h" },
						{ "namespaceGracePeriodDuration", "0h" },
						{ "reservedRootNamespaceNames", "reserved" },

						{ "namespaceRentalFeeSinkPublicKey", "0000000000000000000000000000000000000000000000000000000000000000" },
						{ "rootNamespaceRentalFeePerBlock", "0" },
						{ "childNamespaceRentalFee", "0" },

						{ "maxChildNamespaces", "0" },
						{ "maxMosaicsPerAccount", "0" },

						{ "maxMosaicDuration", "0h" },

						{ "isMosaicLevyUpdateAllowed", "false" },
						{ "maxMosaicDivisibility", "0" },
						{ "maxMosaicDivisibleUnits", "0" },

						{ "mosaicRentalFeeSinkPublicKey", "0000000000000000000000000000000000000000000000000000000000000000" },
						{ "mosaicRentalFee", "0" }
					}
				}}));

				PluginManager manager(config, StorageConfiguration());
				RegisterNamespaceSubsystem(manager);

				// Act:
				action(manager);
			}

		public:
			static std::vector<model::EntityType> GetTransactionTypes() {
				return {
					model::Entity_Type_Register_Namespace,
					model::Entity_Type_Mosaic_Definition,
					model::Entity_Type_Mosaic_Supply_Change
				};
			}

			static std::vector<std::string> GetCacheNames() {
				return { "NamespaceCache", "MosaicCache" };
			}

			static std::vector<ionet::PacketType> GetDiagnosticPacketTypes() {
				return {
					ionet::PacketType::Namespace_Infos,
					ionet::PacketType::Namespace_State_Path,
					ionet::PacketType::Mosaic_Infos,
					ionet::PacketType::Mosaic_State_Path
				};
			}

			static std::vector<std::string> GetDiagnosticCounterNames() {
				return { "NS C", "NS C AS", "NS C DS", "MOSAIC C", "MOSAIC C DS" };
			}

			static std::vector<std::string> GetStatelessValidatorNames() {
				return {
					"NamespaceTypeValidator",
					"NamespaceNameValidator",
					"RootNamespaceValidator",
					"MosaicNameValidator",
					"MosaicPropertiesValidator",
					"MosaicSupplyChangeValidator"
				};
			}

			static std::vector<std::string> GetStatefulValidatorNames() {
				return {
					"RootNamespaceAvailabilityValidator",
					"ChildNamespaceAvailabilityValidator",
					"RootNamespaceMaxChildrenValidator",
					"MosaicChangeAllowedValidator",
					"NamespaceMosaicConsistencyValidator",
					"MosaicAvailabilityValidator",
					"MosaicTransferValidator",
					"MaxMosaicsBalanceTransferValidator",
					"MaxMosaicsSupplyChangeValidator",
					"MosaicSupplyChangeAllowedValidator"
				};
			}

			static std::vector<std::string> GetObserverNames() {
				return {
					"RegisterNamespaceMosaicPruningObserver",
					"RootNamespaceObserver",
					"ChildNamespaceObserver",
					"NamespaceTouchObserver",
					"NamespacePruningObserver",
					"MosaicDefinitionObserver",
					"MosaicSupplyChangeObserver",
					"MosaicTouchObserver",
					"MosaicPruningObserver"
				};
			}

			static std::vector<std::string> GetPermanentObserverNames() {
				return GetObserverNames();
			}
		};
	}

	DEFINE_PLUGIN_TESTS(NamespacePluginTests, NamespacePluginTraits)
}}

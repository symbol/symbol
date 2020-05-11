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

#include "src/plugins/MosaicPlugin.h"
#include "src/cache/MosaicCache.h"
#include "src/model/MosaicEntityType.h"
#include "tests/test/plugins/PluginManagerFactory.h"
#include "tests/test/plugins/PluginTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace plugins {

	namespace {
		struct MosaicPluginTraits {
		public:
			template<typename TAction>
			static void RunTestAfterRegistration(TAction action) {
				// Arrange:
				auto config = model::BlockChainConfiguration::Uninitialized();
				config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(1);
				config.BlockPruneInterval = 150;
				config.Plugins.emplace("catapult.plugins.mosaic", utils::ConfigurationBag({{
					"",
					{
						{ "maxMosaicsPerAccount", "0" },
						{ "maxMosaicDuration", "0h" },
						{ "maxMosaicDivisibility", "0" },

						{ "mosaicRentalFeeSinkAddress", "SAXQUTQQNS6JEJG7PLC6FRVJ2USS44GLMXG4DOA" },
						{ "mosaicRentalFee", "0" }
					}
				}}));

				auto manager = test::CreatePluginManager(config);
				RegisterMosaicSubsystem(manager);

				// Act:
				action(manager);
			}

		public:
			static std::vector<model::EntityType> GetTransactionTypes() {
				return {
					model::Entity_Type_Mosaic_Definition,
					model::Entity_Type_Mosaic_Supply_Change
				};
			}

			static std::vector<std::string> GetCacheNames() {
				return { "MosaicCache" };
			}

			static std::vector<ionet::PacketType> GetNonDiagnosticPacketTypes() {
				return { ionet::PacketType::Mosaic_State_Path };
			}

			static std::vector<ionet::PacketType> GetDiagnosticPacketTypes() {
				return { ionet::PacketType::Mosaic_Infos };
			}

			static std::vector<std::string> GetDiagnosticCounterNames() {
				return { "MOSAIC C" };
			}

			static std::vector<std::string> GetStatelessValidatorNames() {
				return {
					"MosaicFlagsValidator",
					"MosaicIdValidator",
					"MosaicSupplyChangeValidator"
				};
			}

			static std::vector<std::string> GetStatefulValidatorNames() {
				return {
					"RequiredMosaicValidator",
					"MosaicAvailabilityValidator",
					"MosaicDivisibilityValidator",
					"MosaicDurationValidator",
					"MosaicTransferValidator",
					"MaxMosaicsBalanceTransferValidator",
					"MaxMosaicsSupplyChangeValidator",
					"MosaicSupplyChangeAllowedValidator"
				};
			}

			static std::vector<std::string> GetObserverNames() {
				return {
					"MosaicDefinitionObserver",
					"MosaicSupplyChangeObserver",
					"MosaicSupplyInflationObserver",
					"MosaicRentalFeeObserver",
					"MosaicTouchObserver"
				};
			}

			static std::vector<std::string> GetPermanentObserverNames() {
				return GetObserverNames();
			}
		};
	}

	DEFINE_PLUGIN_TESTS(MosaicPluginTests, MosaicPluginTraits)
}}

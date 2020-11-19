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

#include "src/plugins/MetadataPlugin.h"
#include "src/model/MetadataEntityType.h"
#include "tests/test/plugins/PluginManagerFactory.h"
#include "tests/test/plugins/PluginTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace plugins {

	namespace {
		struct MetadataPluginTraits {
		public:
			template<typename TAction>
			static void RunTestAfterRegistration(TAction action) {
				// Arrange:
				auto config = model::BlockChainConfiguration::Uninitialized();
				config.Plugins.emplace("catapult.plugins.metadata", utils::ConfigurationBag({{
					"",
					{
						{ "maxValueSize", "10" }
					}
				}}));

				auto manager = test::CreatePluginManager(config);
				RegisterMetadataSubsystem(manager);

				// Act:
				action(manager);
			}

		public:
			static std::vector<model::EntityType> GetTransactionTypes() {
				return { model::Entity_Type_Account_Metadata, model::Entity_Type_Mosaic_Metadata, model::Entity_Type_Namespace_Metadata };
			}

			static std::vector<std::string> GetCacheNames() {
				return { "MetadataCache" };
			}

			static std::vector<ionet::PacketType> GetNonDiagnosticPacketTypes() {
				return { ionet::PacketType::Metadata_State_Path };
			}

			static std::vector<ionet::PacketType> GetDiagnosticPacketTypes() {
				return { ionet::PacketType::Metadata_Infos };
			}

			static std::vector<std::string> GetDiagnosticCounterNames() {
				return { "METADATA C" };
			}

			static std::vector<std::string> GetStatelessValidatorNames() {
				return { "MetadataSizesValidator" };
			}

			static std::vector<std::string> GetStatefulValidatorNames() {
				return { "MetadataValueValidator" };
			}

			static std::vector<std::string> GetObserverNames() {
				return { "MetadataValueObserver" };
			}

			static std::vector<std::string> GetPermanentObserverNames() {
				return GetObserverNames();
			}
		};
	}

	DEFINE_PLUGIN_TESTS(MetadataPluginTests, MetadataPluginTraits)
}}

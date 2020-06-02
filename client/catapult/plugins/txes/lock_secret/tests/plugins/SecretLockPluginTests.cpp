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

#include "src/plugins/SecretLockPlugin.h"
#include "src/model/SecretLockEntityType.h"
#include "tests/test/plugins/PluginManagerFactory.h"
#include "tests/test/plugins/PluginTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace plugins {

	namespace {
		struct SecretLockPluginTraits {
		public:
			template<typename TAction>
			static void RunTestAfterRegistration(TAction action) {
				// Arrange:
				auto config = model::BlockChainConfiguration::Uninitialized();
				config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(1);
				config.Plugins.emplace("catapult.plugins.locksecret", utils::ConfigurationBag({{
					"",
					{
						{ "maxSecretLockDuration", "30d" },
						{ "minProofSize", "10" },
						{ "maxProofSize", "1000" }
					}
				}}));

				auto manager = test::CreatePluginManager(config);
				RegisterSecretLockSubsystem(manager);

				// Act:
				action(manager);
			}

		public:
			static std::vector<model::EntityType> GetTransactionTypes() {
				return { model::Entity_Type_Secret_Lock, model::Entity_Type_Secret_Proof };
			}

			static std::vector<std::string> GetCacheNames() {
				return { "SecretLockInfoCache" };
			}

			static std::vector<ionet::PacketType> GetNonDiagnosticPacketTypes() {
				return { ionet::PacketType::Secret_Lock_State_Path };
			}

			static std::vector<ionet::PacketType> GetDiagnosticPacketTypes() {
				return { ionet::PacketType::Secret_Lock_Infos };
			}

			static std::vector<std::string> GetDiagnosticCounterNames() {
				return { "SECRETLOCK C" };
			}

			static std::vector<std::string> GetStatelessValidatorNames() {
				return { "SecretLockDurationValidator", "SecretLockHashAlgorithmValidator", "ProofSecretValidator" };
			}

			static std::vector<std::string> GetStatefulValidatorNames() {
				return { "SecretLockCacheUniqueValidator", "ProofValidator" };
			}

			static std::vector<std::string> GetObserverNames() {
				return {
					"SecretLockObserver",
					"ExpiredSecretLockInfoObserver",
					"ProofObserver"
				};
			}

			static std::vector<std::string> GetPermanentObserverNames() {
				return GetObserverNames();
			}
		};
	}

	DEFINE_PLUGIN_TESTS(SecretLockPluginTests, SecretLockPluginTraits)
}}

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

#include "src/plugins/LockPlugin.h"
#include "plugins/txes/lock/src/model/LockEntityType.h"
#include "tests/test/plugins/PluginTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace plugins {

	namespace {
		struct LockPluginTraits {
		public:
			template<typename TAction>
			static void RunTestAfterRegistration(TAction action) {
				// Arrange:
				auto config = model::BlockChainConfiguration::Uninitialized();
				config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(1);
				config.Plugins.emplace("catapult.plugins.lock", utils::ConfigurationBag({{
					"",
					{
						{ "lockedFundsPerAggregate", "10'000'000" },
						{ "maxHashLockDuration", "2d" },
						{ "maxSecretLockDuration", "30d" },
						{ "minProofSize", "10" },
						{ "maxProofSize", "1000" }
					}
				}}));

				PluginManager manager(config, StorageConfiguration());
				RegisterLockSubsystem(manager);

				// Act:
				action(manager);
			}

		public:
			static std::vector<model::EntityType> GetTransactionTypes() {
				return { model::Entity_Type_Hash_Lock, model::Entity_Type_Secret_Lock, model::Entity_Type_Secret_Proof };
			}

			static std::vector<std::string> GetCacheNames() {
				return { "HashLockInfoCache", "SecretLockInfoCache" };
			}

			static std::vector<ionet::PacketType> GetDiagnosticPacketTypes() {
				return {};
			}

			static std::vector<std::string> GetDiagnosticCounterNames() {
				return { "HASHLOCK C", "SECRETLOCK C" };
			}

			static std::vector<std::string> GetStatelessValidatorNames() {
				return {
					"HashLockDurationValidator",
					"HashLockMosaicValidator",
					"SecretLockDurationValidator",
					"SecretLockHashAlgorithmValidator",
					"ProofSecretValidator"
				};
			}

			static std::vector<std::string> GetStatefulValidatorNames() {
				return { "AggregateHashPresentValidator", "HashCacheUniqueValidator", "SecretCacheUniqueValidator", "ProofValidator" };
			}

			static std::vector<std::string> GetObserverNames() {
				return {
					"HashLockObserver",
					"SecretLockObserver",
					"ExpiredHashLockInfoObserver",
					"ExpiredSecretLockInfoObserver",
					"ProofObserver",
					"HashLockInfoPruningObserver",
					"SecretLockInfoPruningObserver",
					"CompletedAggregateObserver"
				};
			}

			static std::vector<std::string> GetPermanentObserverNames() {
				return GetObserverNames();
			}
		};
	}

	DEFINE_PLUGIN_TESTS(LockPluginTests, LockPluginTraits)
}}

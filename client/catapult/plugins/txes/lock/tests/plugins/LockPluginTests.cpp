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

				PluginManager manager(config);
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

	DEFINE_PLUGIN_TESTS(LockPluginTests, LockPluginTraits);
}}

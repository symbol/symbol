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

				PluginManager manager(config);
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
				return {};
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

	DEFINE_PLUGIN_TESTS(MultisigPluginTests, MultisigPluginTraits);
}}

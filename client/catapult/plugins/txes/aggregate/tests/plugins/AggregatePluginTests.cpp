#include "src/plugins/AggregatePlugin.h"
#include "tests/test/plugins/PluginTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace plugins {

	namespace {
		template<bool EnableStrict>
		struct BasicAggregatePluginTraits {
		public:
			template<typename TAction>
			static void RunTestAfterRegistration(TAction action) {
				// Arrange:
				auto config = model::BlockChainConfiguration::Uninitialized();
				config.Plugins.emplace("catapult.plugins.aggregate", utils::ConfigurationBag({{
					"",
					{
						{ "maxTransactionsPerAggregate", "0" },
						{ "maxCosignaturesPerAggregate", "0" },
						{ "enableStrictCosignatureCheck", EnableStrict ? "true" : "false" },
					}
				}}));

				PluginManager manager(config);
				RegisterAggregateSubsystem(manager);

				// Act:
				action(manager);
			}

		public:
			static std::vector<model::EntityType> GetTransactionTypes() {
				return { model::EntityType::Aggregate };
			}

			static std::vector<std::string> GetCacheNames() {
				return {};
			}

			static std::vector<ionet::PacketType> GetDiagnosticPacketTypes() {
				return {};
			}

			static std::vector<std::string> GetDiagnosticCounterNames() {
				return {};
			}

			static std::vector<std::string> GetStatefulValidatorNames() {
				return {};
			}

			static std::vector<std::string> GetObserverNames() {
				return {};
			}

			static std::vector<std::string> GetPermanentObserverNames() {
				return {};
			}
		};

		// notice that the configured stateless validators are config-dependent

		struct AggregatePluginTraits : public BasicAggregatePluginTraits<false> {
		public:
			static std::vector<std::string> GetStatelessValidatorNames() {
				return { "BasicAggregateCosignaturesValidator" };
			}
		};

		struct StrictAggregatePluginTraits : public BasicAggregatePluginTraits<true> {
		public:
			static std::vector<std::string> GetStatelessValidatorNames() {
				return { "BasicAggregateCosignaturesValidator", "StrictAggregateCosignaturesValidator" };
			}
		};
	}

	DEFINE_PLUGIN_TESTS(AggregatePluginTests, AggregatePluginTraits);
	DEFINE_PLUGIN_TESTS(StrictAggregatePluginTests, StrictAggregatePluginTraits);
}}

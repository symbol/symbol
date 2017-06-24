#include "src/CoreSystem.h"
#include "tests/test/plugins/PluginTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace plugins {

	namespace {
		struct CoreSystemTraits {
		public:
			template<typename TAction>
			static void RunTestAfterRegistration(TAction action) {
				// Arrange:
				PluginManager manager(model::BlockChainConfiguration::Uninitialized());
				RegisterCoreSystem(manager);

				// Act:
				action(manager);
			}

		public:
			static std::vector<model::EntityType> GetTransactionTypes() {
				return {};
			}

			static std::vector<std::string> GetCacheNames() {
				return { "AccountStateCache" };
			}

			static std::vector<ionet::PacketType> GetDiagnosticPacketTypes() {
				return { ionet::PacketType::Account_Infos };
			}

			static std::vector<std::string> GetDiagnosticCounterNames() {
				return { "ACNTST C" };
			}

			static std::vector<std::string> GetStatelessValidatorNames() {
				return { "NonFutureBlockValidator", "MaxTransactionsValidator", "NetworkValidator", "SignatureValidator" };
			}

			static std::vector<std::string> GetStatefulValidatorNames() {
				return { "DeadlineValidator", "NemesisSinkValidator", "EligibleHarvesterValidator", "BalanceValidator" };
			}

			static std::vector<std::string> GetObserverNames() {
				return {
					"AccountAddressObserver",
					"AccountPublicKeyObserver",
					"BalanceObserver",
					"HarvestFeeObserver",
					"RecalculateImportancesObserver"
				};
			}

			static std::vector<std::string> GetPermanentObserverNames() {
				return {
					"AccountAddressObserver",
					"AccountPublicKeyObserver",
					"BalanceObserver",
					"HarvestFeeObserver"
				};
			}
		};
	}

	DEFINE_PLUGIN_TESTS(CoreSystemTests, CoreSystemTraits);
}}

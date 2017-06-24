#include "src/plugins/MemoryHashCacheSystem.h"
#include "tests/test/plugins/PluginTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace plugins {

	namespace {
		struct MemoryHashCacheSystemTraits {
		public:
			template<typename TAction>
			static void RunTestAfterRegistration(TAction action) {
				// Arrange:
				PluginManager manager(model::BlockChainConfiguration::Uninitialized());
				RegisterMemoryHashCacheSystem(manager);

				// Act:
				action(manager);
			}

		public:
			static std::vector<model::EntityType> GetTransactionTypes() {
				return {};
			}

			static std::vector<std::string> GetCacheNames() {
				return { "HashCache" };
			}

			static std::vector<ionet::PacketType> GetDiagnosticPacketTypes() {
				return { ionet::PacketType::Confirm_Timestamped_Hashes };
			}

			static std::vector<std::string> GetDiagnosticCounterNames() {
				return { "HASH C" };
			}

			static std::vector<std::string> GetStatelessValidatorNames() {
				return {};
			}

			static std::vector<std::string> GetStatefulValidatorNames() {
				return { "UniqueTransactionHashValidator" };
			}

			static std::vector<std::string> GetObserverNames() {
				return { "TransactionHashObserver", "TransactionHashPruningObserver" };
			}

			static std::vector<std::string> GetPermanentObserverNames() {
				return {};
			}
		};
	}

	DEFINE_PLUGIN_TESTS(MemoryHashCacheSystemTests, MemoryHashCacheSystemTraits);
}}

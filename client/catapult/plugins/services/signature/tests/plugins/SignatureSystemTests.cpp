#include "src/plugins/SignatureSystem.h"
#include "tests/test/plugins/PluginTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace plugins {

	namespace {
		struct SignatureSystemTraits {
		public:
			template<typename TAction>
			static void RunTestAfterRegistration(TAction action) {
				// Arrange:
				PluginManager manager(model::BlockChainConfiguration::Uninitialized());
				RegisterSignatureSystem(manager);

				// Act:
				action(manager);
			}

		public:
			static std::vector<model::EntityType> GetTransactionTypes() {
				return {};
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

			static std::vector<std::string> GetStatelessValidatorNames() {
				return { "SignatureValidator" };
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
	}

	DEFINE_PLUGIN_TESTS(SignatureSystemTests, SignatureSystemTraits);
}}

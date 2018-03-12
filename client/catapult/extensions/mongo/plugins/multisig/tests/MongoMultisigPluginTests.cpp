#include "mongo/tests/test/MongoPluginTestUtils.h"
#include "plugins/txes/multisig/src/model/MultisigEntityType.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		struct MongoMultisigPluginTraits {
		public:
			static constexpr auto RegisterSubsystem = RegisterMongoSubsystem;

			static std::vector<model::EntityType> GetTransactionTypes() {
				return { model::Entity_Type_Modify_Multisig_Account };
			}

			static std::string GetStorageName() {
				return "{ MultisigCache }";
			}
		};
	}

	DEFINE_MONGO_PLUGIN_TESTS(MongoMultisigPluginTests, MongoMultisigPluginTraits);
}}}

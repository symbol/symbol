#include "mongo/tests/test/MongoPluginTestUtils.h"
#include "plugins/txes/lock/src/model/LockEntityType.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		struct MongoLockPluginTraits {
		public:
			static constexpr auto RegisterSubsystem = RegisterMongoSubsystem;

			static std::vector<model::EntityType> GetTransactionTypes() {
				return { model::Entity_Type_Hash_Lock, model::Entity_Type_Secret_Lock, model::Entity_Type_Secret_Proof };
			}

			static std::string GetStorageName() {
				return "{ HashLockInfoCache, SecretLockInfoCache }";
			}
		};
	}

	DEFINE_MONGO_PLUGIN_TESTS(MongoLockPluginTests, MongoLockPluginTraits);
}}}

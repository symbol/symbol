#include "mongo/tests/test/MongoPluginTestUtils.h"
#include "plugins/txes/transfer/src/model/TransferEntityType.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		struct MongoTransferPluginTraits {
		public:
			static constexpr auto RegisterSubsystem = RegisterMongoSubsystem;

			static std::vector<model::EntityType> GetTransactionTypes() {
				return { model::Entity_Type_Transfer };
			}

			static std::string GetStorageName() {
				return "{}";
			}
		};
	}

	DEFINE_MONGO_PLUGIN_TESTS(MongoTransferPluginTests, MongoTransferPluginTraits);
}}}

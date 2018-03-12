#include "mongo/tests/test/MongoPluginTestUtils.h"
#include "plugins/txes/namespace/src/model/MosaicEntityType.h"
#include "plugins/txes/namespace/src/model/NamespaceEntityType.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		struct MongoNamespacePluginTraits {
		public:
			static constexpr auto RegisterSubsystem = RegisterMongoSubsystem;

			static std::vector<model::EntityType> GetTransactionTypes() {
				return {
					model::Entity_Type_Mosaic_Definition,
					model::Entity_Type_Mosaic_Supply_Change,
					model::Entity_Type_Register_Namespace
				};
			}

			static std::string GetStorageName() {
				return "{ NamespaceCache, MosaicCache }";
			}
		};
	}

	DEFINE_MONGO_PLUGIN_TESTS(MongoNamespacePluginTests, MongoNamespacePluginTraits);
}}}

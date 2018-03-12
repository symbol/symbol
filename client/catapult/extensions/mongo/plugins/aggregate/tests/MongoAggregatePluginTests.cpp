#include "mongo/tests/test/MongoPluginTestUtils.h"
#include "plugins/txes/aggregate/src/model/AggregateEntityType.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		struct MongoAggregatePluginTraits {
		public:
			static constexpr auto RegisterSubsystem = RegisterMongoSubsystem;

			static std::vector<model::EntityType> GetTransactionTypes() {
				return { model::Entity_Type_Aggregate_Complete, model::Entity_Type_Aggregate_Bonded };
			}

			static std::string GetStorageName() {
				return "{}";
			}
		};
	}

	DEFINE_MONGO_PLUGIN_TESTS(MongoAggregatePluginTests, MongoAggregatePluginTraits);
}}}

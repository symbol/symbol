#include "src/MosaicSupplyChangeMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/namespace/src/model/MosaicSupplyChangeTransaction.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoTransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MosaicSupplyChangeMapperTests

	namespace {
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS_NO_ADAPT(MosaicSupplyChange);
	}

	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, model::Entity_Type_Mosaic_Supply_Change)

	// region streamTransaction

	PLUGIN_TEST(CanMapSupplyChangeTransaction) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		transaction.MosaicId = MosaicId(753);
		transaction.Direction = model::MosaicSupplyChangeDirection::Increase;
		transaction.Delta = Amount(12349876);

		auto pPlugin = TTraits::CreatePlugin();

		// Act:
		mappers::bson_stream::document builder;
		pPlugin->streamTransaction(builder, transaction);
		auto view = builder.view();

		// Assert:
		EXPECT_EQ(3u, test::GetFieldCount(view));
		EXPECT_EQ(753u, test::GetUint64(view, "mosaicId"));
		EXPECT_EQ(
				model::MosaicSupplyChangeDirection::Increase,
				static_cast<model::MosaicSupplyChangeDirection>(test::GetUint32(view, "direction")));
		EXPECT_EQ(12349876u, test::GetUint64(view, "delta"));
	}

	// endregion
}}}

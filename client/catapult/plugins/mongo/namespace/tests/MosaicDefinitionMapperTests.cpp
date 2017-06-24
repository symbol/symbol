#include "src/MosaicDefinitionMapper.h"
#include "sdk/src/builders/MosaicDefinitionBuilder.h"
#include "plugins/mongo/coremongo/src/mappers/MapperUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/mongo/MapperTestUtils.h"
#include "tests/test/mongo/MongoTransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

#define TEST_CLASS MosaicDefinitionMapperTests

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS(MosaicDefinition);

		using PropertyValuesContainer = model::MosaicProperties::PropertyValuesContainer;

		auto CreateMosaicDefinitionTransaction(const std::string& mosaicName, const model::MosaicProperties& properties) {
			auto signer = test::GenerateKeyPair();
			auto networkId = model::NetworkIdentifier::Mijin_Test;
			builders::MosaicDefinitionBuilder builder(networkId, signer.publicKey(), NamespaceId(123), mosaicName);
			if (properties.is(model::MosaicFlags::Supply_Mutable))
				builder.setSupplyMutable();

			if (properties.is(model::MosaicFlags::Transferable))
				builder.setTransferable();

			if (properties.is(model::MosaicFlags::Levy_Mutable))
				builder.setLevyMutable();

			builder.setDivisibility(properties.divisibility());
			builder.setDuration(properties.duration());
			return builder.build();
		}

		void AssertMosaicDefinitionData(
				NamespaceId parentId,
				MosaicId id,
				const std::string& mosaicName,
				const PropertyValuesContainer& propertyValues,
				size_t numExpectedProperties,
				const bsoncxx::document::view& dbTransaction) {
			// Assert:
			EXPECT_EQ(parentId.unwrap(), test::GetUint64(dbTransaction, "parentId"));
			EXPECT_EQ(id.unwrap(), test::GetUint64(dbTransaction, "mosaicId"));

			auto dbName = dbTransaction["name"].get_binary();
			EXPECT_EQ(mosaicName.size(), dbName.size);
			EXPECT_EQ(
					test::ToHexString(reinterpret_cast<const uint8_t*>(mosaicName.data()), mosaicName.size()),
					test::ToHexString(dbName.bytes, dbName.size));

			auto dbProperties = dbTransaction["properties"].get_array().value;
			ASSERT_EQ(numExpectedProperties, std::distance(dbProperties.cbegin(), dbProperties.cend()));
			auto iter = dbProperties.cbegin();
			for (auto i = 0u; i < numExpectedProperties; ++i, ++iter) {
				auto propertyId = utils::checked_cast<uint32_t, uint8_t>(test::GetUint32(iter->get_document().view(), "id"));
				EXPECT_EQ(i, propertyId) << "id at " << static_cast<uint32_t>(propertyId);
				EXPECT_EQ(propertyValues[propertyId], test::GetUint64(iter->get_document().view(), "value"))
						<< "value at " << static_cast<uint32_t>(propertyId);
			}
		}

		template<typename TTraits>
		void AssertCanMapTransactionWithName(uint64_t duration, size_t numExpectedProperties) {
			std::string mosaicName("jabo38");
			PropertyValuesContainer propertyValues{ { 7, 5, duration } };
			auto properties = model::MosaicProperties::FromValues(propertyValues);
			auto pTransaction = TTraits::Adapt(CreateMosaicDefinitionTransaction(mosaicName, properties));
			auto mosaicId = pTransaction->MosaicId;
			auto pPlugin = TTraits::CreatePlugin();

			// Act:
			mappers::bson_stream::document builder;
			pPlugin->streamTransaction(builder, *pTransaction);
			auto view = builder.view();

			// Assert:
			EXPECT_EQ(4u, test::GetFieldCount(view));
			AssertMosaicDefinitionData(pTransaction->ParentId, mosaicId, mosaicName, propertyValues, numExpectedProperties, view);
		}
	}

	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, model::EntityType::Mosaic_Definition)

	// region streamTransaction

	PLUGIN_TEST(CannotMapMosaicDefinitionTransactionWithoutName) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		transaction.Size = sizeof(typename TTraits::TransactionType);
		transaction.Type = model::EntityType::Mosaic_Definition;
		transaction.MosaicNameSize = 0;

		auto pPlugin = TTraits::CreatePlugin();

		// Act:
		mappers::bson_stream::document builder;
		EXPECT_THROW(pPlugin->streamTransaction(builder, transaction), catapult_runtime_error);
	}

	PLUGIN_TEST(CanMapMosaicDefinitionTransactionWithName_DefaultDuration) {
		// Assert:
		AssertCanMapTransactionWithName<TTraits>(Eternal_Artifact_Duration.unwrap(), 2);
	}

	PLUGIN_TEST(CanMapMosaicDefinitionTransactionWithName_CustomDuration) {
		// Assert:
		AssertCanMapTransactionWithName<TTraits>(321, 3);
	}

	// endregion
}}}

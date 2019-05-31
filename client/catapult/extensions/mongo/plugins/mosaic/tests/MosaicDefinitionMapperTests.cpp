/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "src/MosaicDefinitionMapper.h"
#include "sdk/src/builders/MosaicDefinitionBuilder.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "catapult/constants.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoTransactionPluginTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MosaicDefinitionMapperTests

	namespace {
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS(MosaicDefinition)

		using PropertyValuesContainer = model::MosaicProperties::PropertyValuesContainer;

		model::MosaicFlags GetFlags(const model::MosaicProperties& properties) {
			auto flags = model::MosaicFlags::None;
			auto allFlags = std::initializer_list<model::MosaicFlags>{
				model::MosaicFlags::Supply_Mutable, model::MosaicFlags::Transferable
			};

			for (auto flag : allFlags) {
				if (properties.is(flag))
					flags |= flag;
			}

			return flags;
		}

		auto CreateMosaicDefinitionTransactionBuilder(
				const Key& signer,
				MosaicNonce mosaicNonce,
				const model::MosaicProperties& properties) {
			auto networkId = model::NetworkIdentifier::Mijin_Test;
			builders::MosaicDefinitionBuilder builder(networkId, signer);
			builder.setMosaicNonce(mosaicNonce);
			builder.setFlags(GetFlags(properties));
			builder.setDivisibility(properties.divisibility());
			if (Eternal_Artifact_Duration != properties.duration())
				builder.addProperty({ model::MosaicPropertyId::Duration, properties.duration().unwrap() });

			return builder;
		}

		void AssertMosaicDefinitionData(
				MosaicId id,
				MosaicNonce mosaicNonce,
				const PropertyValuesContainer& propertyValues,
				size_t numExpectedProperties,
				const bsoncxx::document::view& dbTransaction) {
			// Assert:
			EXPECT_EQ(id, MosaicId(test::GetUint64(dbTransaction, "mosaicId")));
			EXPECT_EQ(mosaicNonce, MosaicNonce(test::GetUint32(dbTransaction, "mosaicNonce")));

			auto dbProperties = dbTransaction["properties"].get_array().value;
			ASSERT_EQ(numExpectedProperties, test::GetFieldCount(dbProperties));
			auto iter = dbProperties.cbegin();
			for (auto i = 0u; i < numExpectedProperties; ++i, ++iter) {
				auto propertyId = utils::checked_cast<uint32_t, uint8_t>(test::GetUint32(iter->get_document().view(), "id"));
				EXPECT_EQ(i, propertyId) << "id at " << static_cast<uint32_t>(propertyId);
				EXPECT_EQ(propertyValues[propertyId], test::GetUint64(iter->get_document().view(), "value"))
						<< "value at " << static_cast<uint32_t>(propertyId);
			}
		}

		template<typename TTraits>
		void AssertCanMapTransaction(BlockDuration duration, size_t numExpectedProperties) {
			PropertyValuesContainer propertyValues{ { 3, 5, duration.unwrap() } };
			auto properties = model::MosaicProperties::FromValues(propertyValues);
			auto signer = test::GenerateRandomByteArray<Key>();
			auto mosaicNonce = test::GenerateRandomValue<MosaicNonce>();
			auto pTransaction = TTraits::Adapt(CreateMosaicDefinitionTransactionBuilder(signer, mosaicNonce, properties));
			auto mosaicId = pTransaction->MosaicId;
			auto pPlugin = TTraits::CreatePlugin();

			// Act:
			mappers::bson_stream::document builder;
			pPlugin->streamTransaction(builder, *pTransaction);
			auto view = builder.view();

			// Assert:
			EXPECT_EQ(3u, test::GetFieldCount(view));
			AssertMosaicDefinitionData(mosaicId, mosaicNonce, propertyValues, numExpectedProperties, view);
		}
	}

	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , model::Entity_Type_Mosaic_Definition)

	// region streamTransaction

	PLUGIN_TEST(CanMapMosaicDefinitionTransaction_DefaultDuration) {
		// Assert:
		AssertCanMapTransaction<TTraits>(Eternal_Artifact_Duration, 2);
	}

	PLUGIN_TEST(CanMapMosaicDefinitionTransaction_CustomDuration) {
		// Assert:
		AssertCanMapTransaction<TTraits>(BlockDuration(321), 3);
	}

	// endregion
}}}

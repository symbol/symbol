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
#include "mongo/tests/test/MongoTransactionPluginTests.h"
#include "plugins/txes/mosaic/tests/test/MosaicTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MosaicDefinitionMapperTests

	namespace {
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS(MosaicDefinition)

		auto CreateMosaicDefinitionTransactionBuilder(const Key& signer, MosaicNonce nonce, const model::MosaicProperties& properties) {
			auto networkIdentifier = model::NetworkIdentifier::Private_Test;
			builders::MosaicDefinitionBuilder builder(networkIdentifier, signer);
			builder.setNonce(nonce);
			builder.setFlags(properties.flags());
			builder.setDivisibility(properties.divisibility());
			builder.setDuration(properties.duration());
			return builder;
		}

		void AssertMosaicDefinitionData(
				MosaicId id,
				MosaicNonce nonce,
				const model::MosaicProperties& properties,
				const bsoncxx::document::view& dbTransaction) {
			// Assert:
			EXPECT_EQ(id, MosaicId(test::GetUint64(dbTransaction, "id")));
			EXPECT_EQ(nonce, MosaicNonce(test::GetUint32(dbTransaction, "nonce")));
			EXPECT_EQ(properties.flags(), static_cast<model::MosaicFlags>(test::GetUint8(dbTransaction, "flags")));
			EXPECT_EQ(properties.divisibility(), test::GetUint8(dbTransaction, "divisibility"));
			EXPECT_EQ(properties.duration(), BlockDuration(test::GetUint64(dbTransaction, "duration")));
		}

		template<typename TTraits>
		void AssertCanMapTransaction(BlockDuration duration) {
			auto properties = test::CreateMosaicPropertiesFromValues(4, 5, duration.unwrap());
			auto signer = test::GenerateRandomByteArray<Key>();
			auto mosaicNonce = test::GenerateRandomValue<MosaicNonce>();
			auto pTransaction = TTraits::Adapt(CreateMosaicDefinitionTransactionBuilder(signer, mosaicNonce, properties));
			auto mosaicId = pTransaction->Id;
			auto pPlugin = TTraits::CreatePlugin();

			// Act:
			mappers::bson_stream::document builder;
			pPlugin->streamTransaction(builder, *pTransaction);
			auto view = builder.view();

			// Assert:
			EXPECT_EQ(5u, test::GetFieldCount(view));
			AssertMosaicDefinitionData(mosaicId, mosaicNonce, properties, view);
		}
	}

	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , model::Entity_Type_Mosaic_Definition)

	// region streamTransaction

	PLUGIN_TEST(CanMapMosaicDefinitionTransaction_DefaultDuration) {
		AssertCanMapTransaction<TTraits>(Eternal_Artifact_Duration);
	}

	PLUGIN_TEST(CanMapMosaicDefinitionTransaction_CustomDuration) {
		AssertCanMapTransaction<TTraits>(BlockDuration(321));
	}

	// endregion
}}}

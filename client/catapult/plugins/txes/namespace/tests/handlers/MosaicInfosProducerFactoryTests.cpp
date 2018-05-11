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

#include "src/handlers/MosaicInfosProducerFactory.h"
#include "src/cache/MosaicCache.h"
#include "src/model/MosaicInfo.h"
#include "src/model/MosaicProperty.h"
#include "tests/test/ArtifactTestUtils.h"
#include "tests/test/MosaicTestUtils.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/TestHarness.h"
#include <numeric>

namespace catapult { namespace handlers {

#define TEST_CLASS MosaicInfosProducerFactoryTests

	namespace {
		using MosaicIds = std::vector<MosaicId>;
		using MosaicInfos = std::vector<std::shared_ptr<const model::MosaicInfo>>;

		constexpr size_t Num_Mosaic_Infos = 6;
		constexpr Key Default_Owner{ { 123u } };

		auto PrepareCache() {
			auto pCache = std::make_unique<cache::MosaicCache>(cache::CacheConfiguration());
			auto delta = pCache->createDelta();

			for (auto i = 0u; i < Num_Mosaic_Infos; ++i) {
				auto entry = state::MosaicEntry(
					NamespaceId(i + 10),
					MosaicId(i),
					state::MosaicDefinition(
							Height(i + 20),
							Default_Owner,
							test::CreateMosaicPropertiesWithDuration(BlockDuration(i + 30))));
				entry.increaseSupply(Amount(i + 40));
				delta->insert(entry);
			}

			pCache->commit();
			return pCache;
		}

		void AssertEqual(const MosaicIds& expectedIds, const MosaicInfos& actualInfos) {
			// Assert:
			ASSERT_EQ(expectedIds.size(), actualInfos.size());

			auto i = 0u;
			for (auto id : expectedIds) {
				auto pInfo = actualInfos[i];
				auto rawId = id.unwrap();
				EXPECT_EQ(model::ArtifactInfoAttributes::Is_Known, pInfo->Attributes) << "attributes at " << i;

				EXPECT_EQ(NamespaceId(rawId + 10), pInfo->NamespaceId) << "namespace id at " << i;
				EXPECT_EQ(id, pInfo->Id) << "id at " << i;
				EXPECT_EQ(Amount(rawId + 40), pInfo->Supply) << "supply at " << i;

				++i;
			}
		}

		struct MosaicInfosContext {
		public:
			MosaicInfosContext()
					: pCache(PrepareCache())
					, ProducerFactory(CreateMosaicInfosProducerFactory(*pCache))
			{}

			std::unique_ptr<cache::MosaicCache> pCache;
			MosaicInfosProducerFactory ProducerFactory;
		};

		void AssertCanSupplyMosaicInfos(const std::vector<MosaicId::ValueType>& ids) {
			// Arrange:
			MosaicInfosContext context;
			auto expectedIds = test::ToArtifactIds<MosaicId>(ids);
			auto idRange = model::EntityRange<MosaicId>::CopyFixed(
					reinterpret_cast<const uint8_t*>(expectedIds.data()),
					expectedIds.size());

			// Act:
			auto mosaicInfoProducer = context.ProducerFactory(idRange);

			// Assert:
			AssertEqual(expectedIds, test::ProduceAll(mosaicInfoProducer));
		}
	}

	// region basic

	TEST(TEST_CLASS, CanSupplySingleMosaicInfo) {
		// Assert:
		for (auto i = 0u; i < Num_Mosaic_Infos; ++i)
			AssertCanSupplyMosaicInfos({ i });
	}

	TEST(TEST_CLASS, CanSupplyMultipleMosaicInfos) {
		// Assert:
		AssertCanSupplyMosaicInfos({ 1, 3, 5 });
		AssertCanSupplyMosaicInfos({ 2, 4 });
	}

	TEST(TEST_CLASS, CanSupplyAllMosaicInfos) {
		// Assert:
		auto ids = std::vector<MosaicId::ValueType>(Num_Mosaic_Infos);
		std::iota(ids.begin(), ids.end(), 0);
		AssertCanSupplyMosaicInfos(ids);
	}

	// endregion

	// region unknown mosaic

	TEST(TEST_CLASS, ReturnsInfoForUnkownMosaic) {
		// Arrange: request an unknown mosaic to be supplied
		MosaicInfosContext context;
		MosaicId id(7);
		auto idRange = model::EntityRange<MosaicId>::CopyFixed(reinterpret_cast<const uint8_t*>(&id), 1);

		// Act:
		auto mosaicInfoProducer = context.ProducerFactory(idRange);
		auto mosaicInfos = test::ProduceAll(mosaicInfoProducer);

		// Assert:
		ASSERT_EQ(1u, mosaicInfos.size());

		const auto& pInfo = mosaicInfos[0];
		EXPECT_EQ(MosaicId(7), pInfo->Id);
		EXPECT_EQ(model::ArtifactInfoAttributes::None, pInfo->Attributes);
	}

	// endregion
}}

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

#include "src/mappers/MosaicDescriptorMapper.h"
#include "src/mappers/MosaicDescriptor.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/namespace/src/state/MosaicDefinition.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "plugins/txes/namespace/tests/test/MosaicTestUtils.h"
#include "tests/test/NamespaceMapperTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MosaicDescriptorMapperTests

	// region ToDbModel

	namespace {
		enum class MosaicStatus { Active, Inactive };

		std::shared_ptr<const state::MosaicEntry> CreateMosaicEntry() {
			return test::CreateMosaicEntry(
					NamespaceId(234),
					MosaicId(345),
					Height(123),
					test::GenerateRandomData<Key_Size>(),
					Amount(456),
					BlockDuration(12345));
		}

		MosaicDescriptor CreateMosaicDescriptor(MosaicStatus status) {
			return MosaicDescriptor(CreateMosaicEntry(), 2, MosaicStatus::Active == status);
		}

		void AssertCanMapMosaicDescriptor(MosaicStatus status) {
			// Arrange:
			auto descriptor = CreateMosaicDescriptor(status);

			// Act:
			auto document = ToDbModel(descriptor);
			auto documentView = document.view();

			// Assert:
			EXPECT_EQ(2u, test::GetFieldCount(documentView));

			auto metaView = documentView["meta"].get_document().view();
			EXPECT_EQ(2u, test::GetFieldCount(metaView));
			test::AssertEqualMosaicMetadata(descriptor, metaView);

			auto mosaicView = documentView["mosaic"].get_document().view();
			EXPECT_EQ(7u, test::GetFieldCount(mosaicView));
			test::AssertEqualMosaicData(descriptor, mosaicView);
		}
	}

	TEST(TEST_CLASS, CanMapMosaicDescriptor_ModelToDbModel) {
		// Assert:
		AssertCanMapMosaicDescriptor(MosaicStatus::Inactive);
		AssertCanMapMosaicDescriptor(MosaicStatus::Active);
	}

	// endregion

	// region ToMosaicDescriptor

	namespace {
		bsoncxx::document::value CreateDbMosaicDescriptor(MosaicStatus status) {
			auto descriptor = CreateMosaicDescriptor(status);
			return ToDbModel(descriptor);
		}

		void AssertCanMapDbMosaicDescriptor(MosaicStatus status) {
			// Arrange:
			auto entry = CreateMosaicEntry();
			auto dbDescriptor = CreateDbMosaicDescriptor(status);

			// Act:
			auto descriptor = ToMosaicDescriptor(dbDescriptor);

			// Assert:
			auto view = dbDescriptor.view();
			EXPECT_EQ(2u, test::GetFieldCount(view));

			auto metaView = view["meta"].get_document().view();
			EXPECT_EQ(2u, test::GetFieldCount(metaView));
			test::AssertEqualMosaicMetadata(descriptor, metaView);

			auto mosaicView = view["mosaic"].get_document().view();
			EXPECT_EQ(7u, test::GetFieldCount(mosaicView));
			test::AssertEqualMosaicData(descriptor, mosaicView);
		}
	}

	TEST(TEST_CLASS, CanMapMosaicDescriptor_DbModelToModel) {
		// Assert:
		AssertCanMapDbMosaicDescriptor(MosaicStatus::Inactive);
		AssertCanMapDbMosaicDescriptor(MosaicStatus::Active);
	}

	// endregion
}}}

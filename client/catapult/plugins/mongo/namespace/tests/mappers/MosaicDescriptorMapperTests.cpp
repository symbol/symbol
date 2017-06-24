#include "src/mappers/MosaicDescriptorMapper.h"
#include "plugins/mongo/coremongo/src/mappers/MapperUtils.h"
#include "plugins/mongo/namespace/tests/test/MapperTestUtils.h"
#include "plugins/txes/namespace/src/state/MosaicDefinition.h"
#include "plugins/txes/namespace/src/state/MosaicDescriptor.h"
#include "plugins/txes/namespace/tests/test/MosaicTestUtils.h"
#include "tests/test/mongo/MapperTestUtils.h"
#include "tests/TestHarness.h"

#define TEST_CLASS MosaicDescriptorMapperTests

namespace catapult { namespace mongo { namespace mappers {

	// region ToDbModel

	namespace {
		namespace test = catapult::test;

		enum class MosaicStatus { Active, Inactive };

		std::shared_ptr<const state::MosaicEntry> CreateMosaicEntry() {
			return test::CreateMosaicEntry(
					NamespaceId(234),
					MosaicId(345),
					Height(123),
					test::GenerateRandomData<Key_Size>(),
					Amount(456),
					ArtifactDuration(12345));
		}

		state::MosaicDescriptor CreateMosaicDescriptor(MosaicStatus status) {
			return state::MosaicDescriptor(CreateMosaicEntry(), 2, MosaicStatus::Active == status);
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
			mongo::test::AssertEqualMosaicMetadata(descriptor, metaView);

			auto mosaicView = documentView["mosaic"].get_document().view();
			EXPECT_EQ(7u, test::GetFieldCount(mosaicView));
			mongo::test::AssertEqualMosaicData(descriptor, mosaicView);
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
			mongo::test::AssertEqualMosaicMetadata(descriptor, metaView);

			auto mosaicView = view["mosaic"].get_document().view();
			EXPECT_EQ(7u, test::GetFieldCount(mosaicView));
			mongo::test::AssertEqualMosaicData(descriptor, mosaicView);
		}
	}

	TEST(TEST_CLASS, CanMapMosaicDescriptor_DbModelToModel) {
		// Assert:
		AssertCanMapDbMosaicDescriptor(MosaicStatus::Inactive);
		AssertCanMapDbMosaicDescriptor(MosaicStatus::Active);
	}

	// endregion
}}}

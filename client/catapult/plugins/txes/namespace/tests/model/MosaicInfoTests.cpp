#include "src/model/MosaicInfo.h"
#include "tests/test/core/VariableSizedEntityTestUtils.h"
#include "tests/test/nodeps/NumericTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS MosaicInfoTests

	TEST(TEST_CLASS, EntityHasExpectedSize) {
		// Arrange:
		auto expectedSize =
			sizeof(NamespaceId) // namespace id
			+ sizeof(MosaicId) // id
			+ sizeof(ArtifactInfoAttributes) // attributes
			+ sizeof(Amount); // supply

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(MosaicInfo));
		EXPECT_EQ(25u, sizeof(MosaicInfo));
	}
}}

#include "src/model/NamespaceInfo.h"
#include "tests/test/core/VariableSizedEntityTestUtils.h"
#include "tests/test/nodeps/NumericTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS NamespaceInfoTests

	TEST(TEST_CLASS, EntityHasExpectedSize) {
		// Arrange:
		auto expectedSize =
			sizeof(uint32_t) // size of namespace info
			+ sizeof(NamespaceId) // id
			+ sizeof(ArtifactInfoAttributes) // attributes
			+ sizeof(uint16_t); // number of children

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(NamespaceInfo));
		EXPECT_EQ(15u, sizeof(NamespaceInfo));
	}

	// region CalculateRealSize

	TEST(TEST_CLASS, CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		NamespaceInfo namespaceInfo;
		namespaceInfo.Size = 0;
		namespaceInfo.NumChildren = 100;

		// Act:
		auto realSize = CalculateRealSize(namespaceInfo);

		// Assert:
		EXPECT_EQ(sizeof(NamespaceInfo) + 100 * sizeof(NamespaceId), realSize);
	}

	TEST(TEST_CLASS, CalculateRealSizeDoesNotOverflowWithMaxValues) {
		// Arrange:
		NamespaceInfo namespaceInfo;
		namespaceInfo.Size = 0;
		test::SetMaxValue(namespaceInfo.NumChildren);

		// Act:
		auto realSize = CalculateRealSize(namespaceInfo);

		// Assert:
		EXPECT_EQ(sizeof(NamespaceInfo) + namespaceInfo.NumChildren * sizeof(NamespaceId), realSize);
		EXPECT_GE(std::numeric_limits<uint32_t>::max(), realSize);
	}

	// endregion

	namespace {
		struct NamespaceInfoTraits {
			static auto Create() {
				NamespaceInfo namespaceInfo;
				namespaceInfo.NumChildren = 100;
				return namespaceInfo;
			}

			static auto GenerateEntityWithAttachments(uint16_t count) {
				uint32_t entitySize = sizeof(NamespaceInfo) + count * sizeof(NamespaceId);
				std::unique_ptr<NamespaceInfo> pNamespaceInfo(reinterpret_cast<NamespaceInfo*>(::operator new(entitySize)));
				pNamespaceInfo->Size = entitySize;
				pNamespaceInfo->NumChildren = count;
				return pNamespaceInfo;
			}

			template<typename TEntity>
			static auto GetAttachmentPointer(TEntity& entity) {
				return entity.ChildrenPtr();
			}
		};
	}

	DEFINE_IS_SIZE_VALID_TESTS(TEST_CLASS, NamespaceInfoTraits) // IsSizeValid
	DEFINE_ATTACHMENT_POINTER_TESTS(TEST_CLASS, NamespaceInfoTraits) // ChildrenPtr
}}

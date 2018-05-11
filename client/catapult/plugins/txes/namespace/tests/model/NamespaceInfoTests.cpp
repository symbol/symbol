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

#include "src/model/NamespaceInfo.h"
#include "catapult/utils/MemoryUtils.h"
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
		namespaceInfo.ChildCount = 100;

		// Act:
		auto realSize = NamespaceInfo::CalculateRealSize(namespaceInfo);

		// Assert:
		EXPECT_EQ(sizeof(NamespaceInfo) + 100 * sizeof(NamespaceId), realSize);
	}

	TEST(TEST_CLASS, CalculateRealSizeDoesNotOverflowWithMaxValues) {
		// Arrange:
		NamespaceInfo namespaceInfo;
		namespaceInfo.Size = 0;
		test::SetMaxValue(namespaceInfo.ChildCount);

		// Act:
		auto realSize = NamespaceInfo::CalculateRealSize(namespaceInfo);

		// Assert:
		EXPECT_EQ(sizeof(NamespaceInfo) + namespaceInfo.ChildCount * sizeof(NamespaceId), realSize);
		EXPECT_GE(std::numeric_limits<uint32_t>::max(), realSize);
	}

	// endregion

	namespace {
		struct NamespaceInfoTraits {
			static auto GenerateEntityWithAttachments(uint16_t count) {
				uint32_t entitySize = sizeof(NamespaceInfo) + count * sizeof(NamespaceId);
				auto pNamespaceInfo = utils::MakeUniqueWithSize<NamespaceInfo>(entitySize);
				pNamespaceInfo->Size = entitySize;
				pNamespaceInfo->ChildCount = count;
				return pNamespaceInfo;
			}

			template<typename TEntity>
			static auto GetAttachmentPointer(TEntity& entity) {
				return entity.ChildrenPtr();
			}
		};
	}

	DEFINE_ATTACHMENT_POINTER_TESTS(TEST_CLASS, NamespaceInfoTraits) // ChildrenPtr
}}

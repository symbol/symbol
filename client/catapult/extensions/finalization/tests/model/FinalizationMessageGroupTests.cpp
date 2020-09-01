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

#include "finalization/src/model/FinalizationMessageGroup.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/VariableSizedEntityTestUtils.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/test/nodeps/NumericTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS FinalizationMessageGroupTests

	// region size + alignment

#define MESSAGE_GROUP_FIELDS FIELD(HashesCount) FIELD(SignaturesCount) FIELD(Stage) FIELD(Height)

	TEST(TEST_CLASS, FinalizationMessageGroupHasExpectedSize) {
		// Arrange:
		auto expectedSize = sizeof(SizePrefixedEntity);

#define FIELD(X) expectedSize += sizeof(FinalizationMessageGroup::X);
		MESSAGE_GROUP_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(FinalizationMessageGroup));
		EXPECT_EQ(4 + 20u, sizeof(FinalizationMessageGroup));
	}

	TEST(TEST_CLASS, FinalizationMessageGroupHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(FinalizationMessageGroup, X);
		MESSAGE_GROUP_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(FinalizationMessageGroup) % 8);
	}

#undef MESSAGE_GROUP_FIELDS

	// endregion

	// region CalculateRealSize

	TEST(TEST_CLASS, CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		FinalizationMessageGroup messageGroup;
		messageGroup.Size = 0;
		messageGroup.HashesCount = 67;
		messageGroup.SignaturesCount = 12;

		// Act:
		auto realSize = FinalizationMessageGroup::CalculateRealSize(messageGroup);

		// Assert:
		EXPECT_EQ(sizeof(FinalizationMessageGroup) + 67 * Hash256::Size + 12 * sizeof(crypto::OtsTreeSignature), realSize);
	}

	TEST(TEST_CLASS, CalculateRealSizeDoesNotOverflowWithMaxValues) {
		// Arrange:
		FinalizationMessageGroup messageGroup;
		messageGroup.Size = 0;
		test::SetMaxValue(messageGroup.HashesCount);
		test::SetMaxValue(messageGroup.SignaturesCount);

		// Act:
		auto realSize = FinalizationMessageGroup::CalculateRealSize(messageGroup);

		// Assert:
		auto expectedSize =
				sizeof(FinalizationMessageGroup)
				+ messageGroup.HashesCount * Hash256::Size
				+ messageGroup.SignaturesCount * sizeof(crypto::OtsTreeSignature);
		EXPECT_EQ(expectedSize, realSize);
		EXPECT_GE(std::numeric_limits<uint64_t>::max(), realSize);
	}

	// endregion

	// region data pointers

	namespace {
		struct FinalizationMessageGroupTraits {
			static auto GenerateEntityWithAttachments(uint16_t numHashes, uint16_t numSignatures) {
				uint32_t entitySize = SizeOf32<FinalizationMessageGroup>()
						+ numHashes * static_cast<uint32_t>(Hash256::Size)
						+ numSignatures * SizeOf32<crypto::OtsTreeSignature>();
				auto pMessageGroup = utils::MakeUniqueWithSize<FinalizationMessageGroup>(entitySize);
				pMessageGroup->Size = entitySize;
				pMessageGroup->HashesCount = numHashes;
				pMessageGroup->SignaturesCount = numSignatures;
				return pMessageGroup;
			}

			static constexpr size_t GetAttachment1Size(uint8_t numHashes) {
				return numHashes * Hash256::Size;
			}

			template<typename TEntity>
			static auto GetAttachmentPointer1(TEntity& entity) {
				return entity.HashesPtr();
			}

			template<typename TEntity>
			static auto GetAttachmentPointer2(TEntity& entity) {
				return entity.SignaturesPtr();
			}
		};
	}

	DEFINE_DUAL_ATTACHMENT_POINTER_TESTS(TEST_CLASS, FinalizationMessageGroupTraits)

	// endregion
}}

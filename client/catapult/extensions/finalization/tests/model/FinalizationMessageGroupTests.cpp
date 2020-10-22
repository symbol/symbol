/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#define TEST_CLASS_V1 FinalizationMessageGroupTests_V1

	// region traits

	namespace {
		struct CurrentTraits {
			using TreeSignature = crypto::BmTreeSignature;

			static constexpr uint16_t Signature_Scheme = 1;

			template<typename TMessageGroup>
			static auto GetSignaturesPtr(TMessageGroup& messageGroup) {
				return messageGroup.SignaturesPtr();
			}
		};

		struct V1Traits {
			using TreeSignature = crypto::BmTreeSignatureV1;

			static constexpr uint16_t Signature_Scheme = 0;

			template<typename TMessageGroup>
			static auto GetSignaturesPtr(TMessageGroup& messageGroup) {
				return messageGroup.SignaturesV1Ptr();
			}
		};
	}

#define VERSION_TRAITS(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CurrentTraits>(); } \
	TEST(TEST_CLASS_V1, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<V1Traits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region size + alignment

#define MESSAGE_GROUP_FIELDS FIELD(HashesCount) FIELD(SignaturesCount) FIELD(SignatureScheme) FIELD(Stage) FIELD(Height)

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

	VERSION_TRAITS(CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		FinalizationMessageGroup messageGroup;
		messageGroup.Size = 0;
		messageGroup.SignatureScheme = TTraits::Signature_Scheme;
		messageGroup.HashesCount = 67;
		messageGroup.SignaturesCount = 12;

		// Act:
		auto realSize = FinalizationMessageGroup::CalculateRealSize(messageGroup);

		// Assert:
		EXPECT_EQ(sizeof(FinalizationMessageGroup) + 67 * Hash256::Size + 12 * sizeof(typename TTraits::TreeSignature), realSize);
	}

	VERSION_TRAITS(CalculateRealSizeDoesNotOverflowWithMaxValues) {
		// Arrange:
		FinalizationMessageGroup messageGroup;
		messageGroup.Size = 0;
		messageGroup.SignatureScheme = TTraits::Signature_Scheme;
		test::SetMaxValue(messageGroup.HashesCount);
		test::SetMaxValue(messageGroup.SignaturesCount);

		// Act:
		auto realSize = FinalizationMessageGroup::CalculateRealSize(messageGroup);

		// Assert:
		auto expectedSize =
				sizeof(FinalizationMessageGroup)
				+ messageGroup.HashesCount * Hash256::Size
				+ messageGroup.SignaturesCount * sizeof(typename TTraits::TreeSignature);
		EXPECT_EQ(expectedSize, realSize);
		EXPECT_GE(std::numeric_limits<uint64_t>::max(), realSize);
	}

	// endregion

	// region data pointers

	namespace {
		template<typename TTraits>
		struct FinalizationMessageGroupTraits {
			static auto GenerateEntityWithAttachments(uint16_t numHashes, uint16_t numSignatures) {
				uint32_t entitySize = SizeOf32<FinalizationMessageGroup>()
						+ numHashes * static_cast<uint32_t>(Hash256::Size)
						+ numSignatures * SizeOf32<typename TTraits::TreeSignature>();
				auto pMessageGroup = utils::MakeUniqueWithSize<FinalizationMessageGroup>(entitySize);
				pMessageGroup->Size = entitySize;
				pMessageGroup->SignatureScheme = TTraits::Signature_Scheme;
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
				return TTraits::GetSignaturesPtr(entity);
			}
		};
	}

	DEFINE_DUAL_ATTACHMENT_POINTER_TESTS(TEST_CLASS, FinalizationMessageGroupTraits<CurrentTraits>)
	DEFINE_DUAL_ATTACHMENT_POINTER_TESTS(TEST_CLASS_V1, FinalizationMessageGroupTraits<V1Traits>)

	// endregion
}}

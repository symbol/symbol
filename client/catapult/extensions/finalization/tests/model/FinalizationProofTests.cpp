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

#include "finalization/src/model/FinalizationProof.h"
#include "finalization/src/model/FinalizationProofUtils.h"
#include "finalization/tests/test/FinalizationMessageTestUtils.h"
#include "tests/test/core/SizePrefixedEntityContainerTestUtils.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS FinalizationProofTests

	// region size + alignment

#define PROOF_FIELDS FIELD(Version) FIELD(Round) FIELD(Height) FIELD(Hash)

	TEST(TEST_CLASS, ProofHasExpectedSize) {
		// Arrange:
		auto expectedSize = sizeof(SizePrefixedEntity);

#define FIELD(X) expectedSize += SizeOf32<decltype(FinalizationProof::X)>();
		PROOF_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(FinalizationProofHeader));
		EXPECT_EQ(4u + 52, sizeof(FinalizationProofHeader));

		EXPECT_EQ(sizeof(FinalizationProofHeader), sizeof(FinalizationProof));
	}

	TEST(TEST_CLASS, ProofHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(FinalizationProof, X);
		PROOF_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(FinalizationProofHeader) % 8);
	}

#undef PROOF_FIELDS

	// endregion

	// region test utils

	namespace {
		std::unique_ptr<FinalizationProof> CreateProofWithMessageGroups(size_t numMessageGroups = 3) {
			std::vector<std::shared_ptr<const FinalizationMessage>> messages;
			for (auto i = 0u; i < numMessageGroups; ++i) {
				auto pMessage = test::CreateMessage(Height(100 + i), test::GenerateRandomByteArray<Hash256>());
				pMessage->Data().StepIdentifier = StepIdentifier();
				messages.push_back(std::move(pMessage));
			}

			return CreateFinalizationProof(FinalizationStatistics(), messages);
		}

		std::unique_ptr<FinalizationProof> CreateProofWithReportedSize(uint32_t size) {
			auto pProof = CreateProofWithMessageGroups();
			pProof->Size = size;
			return pProof;
		}

		FinalizationMessageGroup& GetSecondMessageGroup(FinalizationProof& proof) {
			uint8_t* pBytes = reinterpret_cast<uint8_t*>(proof.MessageGroupsPtr());
			auto firstMessageGroupSize = proof.MessageGroupsPtr()->Size;
			return *reinterpret_cast<FinalizationMessageGroup*>(pBytes + firstMessageGroupSize);
		}
	}

	// endregion

	// region messageGroups

	namespace {
		using ConstTraits = test::ConstTraitsT<FinalizationProof>;
		using NonConstTraits = test::NonConstTraitsT<FinalizationProof>;
	}

#define DATA_POINTER_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Const) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ConstTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_NonConst) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NonConstTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	DATA_POINTER_TEST(MessageGroupsAreInaccessibleWhenProofHasNoMessageGroups) {
		// Arrange:
		auto pProof = CreateProofWithReportedSize(sizeof(FinalizationProofHeader));
		auto& accessor = TTraits::GetAccessor(*pProof);

		// Act + Assert:
		EXPECT_FALSE(!!accessor.MessageGroupsPtr());
		EXPECT_EQ(0u, test::CountContainerEntities(accessor.MessageGroups()));
	}

	DATA_POINTER_TEST(MessageGroupsAreAccessibleWhenProofHasMessageGroups) {
		// Arrange:
		auto pProof = CreateProofWithMessageGroups();
		const auto* pProofEnd = test::AsVoidPointer(pProof.get() + 1);
		auto& accessor = TTraits::GetAccessor(*pProof);

		// Act + Assert:
		EXPECT_EQ(pProofEnd, accessor.MessageGroupsPtr());
		EXPECT_EQ(3u, test::CountContainerEntities(accessor.MessageGroups()));
	}

	// endregion

	// region GetMessageGroupPayloadSize

	TEST(TEST_CLASS, GetMessageGroupPayloadSizeReturnsCorrectPayloadSize) {
		// Arrange:
		FinalizationProofHeader header;
		header.Size = sizeof(FinalizationProofHeader) + 123;

		// Act:
		auto payloadSize = GetMessageGroupPayloadSize(header);

		// Assert:
		EXPECT_EQ(123u, payloadSize);
	}

	// endregion

	// region IsSizeValid - no message groups

	TEST(TEST_CLASS, SizeInvalidWhenReportedSizeIsZero) {
		// Arrange:
		auto pProof = CreateProofWithReportedSize(0);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pProof));
	}

	TEST(TEST_CLASS, SizeInvalidWhenReportedSizeIsLessThanHeaderSize) {
		// Arrange:
		auto pProof = CreateProofWithReportedSize(sizeof(FinalizationProofHeader) - 1);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pProof));
	}

	TEST(TEST_CLASS, SizeValidWhenReportedSizeIsEqualToHeaderSize) {
		// Arrange:
		auto pProof = CreateProofWithReportedSize(sizeof(FinalizationProofHeader));

		// Act + Assert:
		EXPECT_TRUE(IsSizeValid(*pProof));
	}

	// endregion

	// region IsSizeValid - invalid inner message groups

	TEST(TEST_CLASS, SizeInvalidWhenAnyMessageGroupHasPartialHeader) {
		// Arrange: create a proof with 1 extra byte (which should be interpeted as a partial tx header)
		auto pProof = CreateProofWithReportedSize(sizeof(FinalizationProofHeader) + 1);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pProof));
	}

	TEST(TEST_CLASS, SizeInvalidWhenAnyMessageGroupHasInvalidSize) {
		// Arrange:
		auto pProof = CreateProofWithMessageGroups();
		++GetSecondMessageGroup(*pProof).SignaturesCount;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pProof));
	}

	TEST(TEST_CLASS, SizeInvalidWhenAnyMessageGroupHasZeroSize) {
		// Arrange:
		auto pProof = CreateProofWithMessageGroups();
		GetSecondMessageGroup(*pProof).Size = 0;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pProof));
	}

	TEST(TEST_CLASS, SizeInvalidWhenAnyInnerMessageGroupExpandsBeyondBuffer) {
		// Arrange:
		auto pProof = CreateProofWithMessageGroups();
		GetSecondMessageGroup(*pProof).Size = pProof->Size - pProof->MessageGroupsPtr()->Size + 1;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pProof));
	}

	// endregion

	// region IsSizeValid - valid message groups

	TEST(TEST_CLASS, SizeInvalidWhenProofWithMessageGroupsHasLargerReportedSizeThanActual) {
		// Arrange:
		auto pProof = CreateProofWithMessageGroups();
		++pProof->Size;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pProof));
	}

	TEST(TEST_CLASS, SizeValidWhenReportedSizeIsEqualToHeaderSizePlusMessageGroupsSize) {
		// Arrange:
		auto pProof = CreateProofWithMessageGroups();

		// Act + Assert:
		EXPECT_TRUE(IsSizeValid(*pProof));
	}

	// endregion
}}

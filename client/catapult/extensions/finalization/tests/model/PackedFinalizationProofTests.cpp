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

#include "finalization/src/model/PackedFinalizationProof.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/VariableSizedEntityTestUtils.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/test/nodeps/NumericTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS PackedFinalizationProofTests

	// region VoteProof (size + alignment)

#define VOTE_FIELDS FIELD(Signature)

	TEST(TEST_CLASS, VoteProofHasExpectedSize) {
		// Arrange:
		auto expectedSize = 0u;

#define FIELD(X) expectedSize += SizeOf32<decltype(VoteProof::X)>();
		VOTE_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(VoteProof));
		EXPECT_EQ(384u, sizeof(VoteProof));
	}

	TEST(TEST_CLASS, VoteProofHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(VoteProof, X);
		VOTE_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(VoteProof) % 8);
	}

#undef VOTE_FIELDS

	// endregion

	namespace {
		// region test utils - packed finalization proof

		std::unique_ptr<PackedFinalizationProof> CreateProof(uint32_t numVotes) {
			uint32_t proofSize = SizeOf32<PackedFinalizationProof>() + numVotes * SizeOf32<VoteProof>();
			auto pProof = utils::MakeUniqueWithSize<PackedFinalizationProof>(proofSize);
			test::FillWithRandomData({ reinterpret_cast<uint8_t*>(pProof.get()), proofSize });
			pProof->Size = proofSize;
			pProof->VoteProofsCount = numVotes;
			return pProof;
		}

		// endregion
	}

	// region PackedFinalizationProof (size + alignment)

#define PROOF_FIELDS FIELD(VoteProofsCount) FIELD(FinalizedHash) FIELD(FinalizedHeight) FIELD(StepIdentifier)

	TEST(TEST_CLASS, PackedFinalizationProofHasExpectedSize) {
		// Arrange:
		auto expectedSize = SizeOf32<TrailingVariableDataLayout<PackedFinalizationProof, VoteProof>>();

#define FIELD(X) expectedSize += SizeOf32<decltype(PackedFinalizationProof::X)>();
		PROOF_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(PackedFinalizationProof));
		EXPECT_EQ(4u + 68, sizeof(PackedFinalizationProof));
	}

	TEST(TEST_CLASS, PackedFinalizationProofHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(PackedFinalizationProof, X);
		PROOF_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(PackedFinalizationProof) % 8);
	}

#undef PROOF_FIELDS

	// endregion

	// region PackedFinalizationProof (CalculateRealSize)

	TEST(TEST_CLASS, CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		PackedFinalizationProof message;
		message.Size = 0;
		message.VoteProofsCount = 67;

		// Act:
		auto realSize = PackedFinalizationProof::CalculateRealSize(message);

		// Assert:
		EXPECT_EQ(sizeof(PackedFinalizationProof) + 67 * sizeof(VoteProof), realSize);
	}

	TEST(TEST_CLASS, CalculateRealSizeDoesNotOverflowWithMaxValues) {
		// Arrange:
		PackedFinalizationProof message;
		message.Size = 0;
		test::SetMaxValue(message.VoteProofsCount);

		// Act:
		auto realSize = PackedFinalizationProof::CalculateRealSize(message);

		// Assert:
		EXPECT_EQ(sizeof(PackedFinalizationProof) + message.VoteProofsCount * sizeof(VoteProof), realSize);
		EXPECT_GE(std::numeric_limits<uint64_t>::max(), realSize);
	}

	// endregion

	// region PackedFinalizationProof (data pointers)

	namespace {
		struct PackedFinalizationProofTraits {
			static constexpr auto GenerateEntityWithAttachments = CreateProof;

			template<typename TEntity>
			static auto GetAttachmentPointer(TEntity& entity) {
				return entity.VoteProofsPtr();
			}
		};
	}

	DEFINE_ATTACHMENT_POINTER_TESTS(TEST_CLASS, PackedFinalizationProofTraits) // VoteProofsPtr

	// endregion
}}

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

#include "finalization/src/model/FinalizationProofUtils.h"
#include "finalization/tests/test/FinalizationMessageTestUtils.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS FinalizationProofUtilsTests

	namespace {
		// region test utils

		size_t CalculateMessageGroupsSize(size_t numMessageGroups, size_t numHashes, size_t numSignatures) {
			return numMessageGroups * sizeof(FinalizationMessageGroup)
					+ numHashes * Hash256::Size
					+ numSignatures * sizeof(crypto::OtsTreeSignature);
		}

		void AssertEqualStatistics(const FinalizationStatistics& expectedStatistics, const FinalizationProof& proof) {
			EXPECT_EQ(expectedStatistics.Point, proof.Point);
			EXPECT_EQ(expectedStatistics.Height, proof.Height);
			EXPECT_EQ(expectedStatistics.Hash, proof.Hash);

			EXPECT_EQ(1u, proof.Version);
		}

		void AssertMessageGroup(
				const FinalizationMessageGroup& messageGroup,
				FinalizationStage expectedStage,
				Height expectedHeight,
				const std::vector<Hash256>& expectedHashes,
				const std::vector<crypto::OtsTreeSignature>& expectedSignatures,
				const std::string& message) {
			ASSERT_EQ(CalculateMessageGroupsSize(1, expectedHashes.size(), expectedSignatures.size()), messageGroup.Size) << message;
			ASSERT_EQ(expectedHashes.size(), messageGroup.HashesCount) << message;
			ASSERT_EQ(expectedSignatures.size(), messageGroup.SignaturesCount) << message;

			EXPECT_EQ(expectedStage, messageGroup.Stage) << message;
			EXPECT_EQ(expectedHeight, messageGroup.Height) << message;

			for (auto i = 0u; i < expectedHashes.size(); ++i)
				EXPECT_EQ(expectedHashes[i], messageGroup.HashesPtr()[i]) << message << " hash at " << i;

			for (auto i = 0u; i < expectedSignatures.size(); ++i)
				EXPECT_EQ(expectedSignatures[i], messageGroup.SignaturesPtr()[i]) << message << " signature at " << i;
		}

		// endregion
	}

	// region zero and one message

	TEST(TEST_CLASS, CanCreateFinalizationProofWithNoMessages) {
		// Arrange:
		auto statistics = FinalizationStatistics{ FinalizationPoint(9), Height(111), test::GenerateRandomByteArray<Hash256>() };

		// Act:
		auto pProof = CreateFinalizationProof(statistics, {});

		// Assert:
		ASSERT_EQ(sizeof(FinalizationProof), pProof->Size);
		AssertEqualStatistics(statistics, *pProof);
	}

	TEST(TEST_CLASS, CanCreateFinalizationProofWithSingleMessage) {
		// Arrange:
		auto statistics = FinalizationStatistics{ FinalizationPoint(9), Height(111), test::GenerateRandomByteArray<Hash256>() };

		auto pMessage = utils::UniqueToShared(test::CreateMessage(Height(11), 3));
		pMessage->StepIdentifier = { FinalizationPoint(9), FinalizationStage::Prevote };

		// Act:
		auto pProof = CreateFinalizationProof(statistics, { pMessage });

		// Assert:
		ASSERT_EQ(sizeof(FinalizationProof) + CalculateMessageGroupsSize(1, 3, 1), pProof->Size);
		AssertEqualStatistics(statistics, *pProof);

		auto messageGroups = pProof->MessageGroups();
		auto messageGroupsIter = messageGroups.cbegin();
		AssertMessageGroup(
				*messageGroupsIter,
				FinalizationStage::Prevote,
				Height(11),
				{ pMessage->HashesPtr()[0], pMessage->HashesPtr()[1], pMessage->HashesPtr()[2] },
				{ pMessage->Signature },
				"group 1");
	}

	// endregion

	// region two messages, reducible

	TEST(TEST_CLASS, CanCreateFinalizationProofWithMultipleMessages_DifferentSignature) {
		// Arrange:
		auto statistics = FinalizationStatistics{ FinalizationPoint(9), Height(111), test::GenerateRandomByteArray<Hash256>() };

		auto pMessage1 = utils::UniqueToShared(test::CreateMessage(Height(11), 3));
		pMessage1->StepIdentifier = { FinalizationPoint(9), FinalizationStage::Prevote };

		auto pMessage2 = utils::UniqueToShared(test::CopyEntity(*pMessage1));
		test::FillWithRandomData(pMessage2->Signature);

		// Act:
		auto pProof = CreateFinalizationProof(statistics, { pMessage1, pMessage2 });

		// Assert:
		ASSERT_EQ(sizeof(FinalizationProof) + CalculateMessageGroupsSize(1, 3, 2), pProof->Size);
		AssertEqualStatistics(statistics, *pProof);

		auto messageGroups = pProof->MessageGroups();
		auto messageGroupsIter = messageGroups.cbegin();
		AssertMessageGroup(
				*messageGroupsIter,
				FinalizationStage::Prevote,
				Height(11),
				{ pMessage1->HashesPtr()[0], pMessage1->HashesPtr()[1], pMessage1->HashesPtr()[2] },
				{ pMessage1->Signature, pMessage2->Signature },
				"group 1");
	}

	// endregion

	// region two messages, irreducible

	TEST(TEST_CLASS, CanCreateFinalizationProofWithMultipleMessages_DifferentStage) {
		// Arrange:
		auto statistics = FinalizationStatistics{ FinalizationPoint(9), Height(111), test::GenerateRandomByteArray<Hash256>() };

		auto pMessage1 = utils::UniqueToShared(test::CreateMessage(Height(11), 3));
		pMessage1->StepIdentifier = { FinalizationPoint(9), FinalizationStage::Prevote };

		auto pMessage2 = utils::UniqueToShared(test::CopyEntity(*pMessage1));
		pMessage2->StepIdentifier.Stage = FinalizationStage::Precommit;

		// Act:
		auto pProof = CreateFinalizationProof(statistics, { pMessage1, pMessage2 });

		// Assert:
		ASSERT_EQ(sizeof(FinalizationProof) + CalculateMessageGroupsSize(2, 6, 2), pProof->Size);
		AssertEqualStatistics(statistics, *pProof);

		auto messageGroups = pProof->MessageGroups();
		auto messageGroupsIter = messageGroups.cbegin();
		AssertMessageGroup(
				*messageGroupsIter,
				FinalizationStage::Prevote,
				Height(11),
				{ pMessage1->HashesPtr()[0], pMessage1->HashesPtr()[1], pMessage1->HashesPtr()[2] },
				{ pMessage1->Signature },
				"group 1");

		// - in the second group, only stage differs (use pMessage1->HashesPtr for emphasis)
		++messageGroupsIter;
		AssertMessageGroup(
				*messageGroupsIter,
				FinalizationStage::Precommit,
				Height(11),
				{ pMessage1->HashesPtr()[0], pMessage1->HashesPtr()[1], pMessage1->HashesPtr()[2] },
				{ pMessage2->Signature },
				"group 2");
	}

	TEST(TEST_CLASS, CanCreateFinalizationProofWithMultipleMessages_DifferentHeight) {
		// Arrange:
		auto statistics = FinalizationStatistics{ FinalizationPoint(9), Height(111), test::GenerateRandomByteArray<Hash256>() };

		auto pMessage1 = utils::UniqueToShared(test::CreateMessage(Height(11), 3));
		pMessage1->StepIdentifier = { FinalizationPoint(9), FinalizationStage::Prevote };

		auto pMessage2 = utils::UniqueToShared(test::CopyEntity(*pMessage1));
		pMessage2->Height = Height(22);

		// Act:
		auto pProof = CreateFinalizationProof(statistics, { pMessage1, pMessage2 });

		// Assert:
		ASSERT_EQ(sizeof(FinalizationProof) + CalculateMessageGroupsSize(2, 6, 2), pProof->Size);
		AssertEqualStatistics(statistics, *pProof);

		auto messageGroups = pProof->MessageGroups();
		auto messageGroupsIter = messageGroups.cbegin();
		AssertMessageGroup(
				*messageGroupsIter,
				FinalizationStage::Prevote,
				Height(11),
				{ pMessage1->HashesPtr()[0], pMessage1->HashesPtr()[1], pMessage1->HashesPtr()[2] },
				{ pMessage1->Signature },
				"group 1");

		// - in the second group, only height differs (use pMessage1->HashesPtr for emphasis)
		++messageGroupsIter;
		AssertMessageGroup(
				*messageGroupsIter,
				FinalizationStage::Prevote,
				Height(22),
				{ pMessage1->HashesPtr()[0], pMessage1->HashesPtr()[1], pMessage1->HashesPtr()[2] },
				{ pMessage2->Signature },
				"group 2");
	}

	TEST(TEST_CLASS, CanCreateFinalizationProofWithMultipleMessages_DifferentHashesContents) {
		// Arrange:
		auto statistics = FinalizationStatistics{ FinalizationPoint(9), Height(111), test::GenerateRandomByteArray<Hash256>() };

		auto pMessage1 = utils::UniqueToShared(test::CreateMessage(Height(11), 3));
		pMessage1->StepIdentifier = { FinalizationPoint(9), FinalizationStage::Prevote };
		pMessage1->HashesPtr()[2] = { { 1 } };

		auto pMessage2 = utils::UniqueToShared(test::CopyEntity(*pMessage1));
		pMessage2->HashesPtr()[2] = { { 2 } };

		// Act:
		auto pProof = CreateFinalizationProof(statistics, { pMessage1, pMessage2 });

		// Assert:
		ASSERT_EQ(sizeof(FinalizationProof) + CalculateMessageGroupsSize(2, 6, 2), pProof->Size);
		AssertEqualStatistics(statistics, *pProof);

		auto messageGroups = pProof->MessageGroups();
		auto messageGroupsIter = messageGroups.cbegin();
		AssertMessageGroup(
				*messageGroupsIter,
				FinalizationStage::Prevote,
				Height(11),
				{ pMessage1->HashesPtr()[0], pMessage1->HashesPtr()[1], pMessage1->HashesPtr()[2] },
				{ pMessage1->Signature },
				"group 1");

		// - in the second group, only last hash differs (use pMessage1->HashesPtr for emphasis)
		++messageGroupsIter;
		AssertMessageGroup(
				*messageGroupsIter,
				FinalizationStage::Prevote,
				Height(11),
				{ pMessage1->HashesPtr()[0], pMessage1->HashesPtr()[1], pMessage2->HashesPtr()[2] },
				{ pMessage2->Signature },
				"group 2");
	}

	TEST(TEST_CLASS, CanCreateFinalizationProofWithMultipleMessages_DifferentHashesNumber) {
		// Arrange:
		auto statistics = FinalizationStatistics{ FinalizationPoint(9), Height(111), test::GenerateRandomByteArray<Hash256>() };

		auto pMessage1 = utils::UniqueToShared(test::CreateMessage(Height(11), 3));
		pMessage1->StepIdentifier = { FinalizationPoint(9), FinalizationStage::Prevote };

		auto pMessage2 = utils::UniqueToShared(test::CopyEntity(*pMessage1));
		pMessage1->Size = static_cast<uint32_t>(pMessage1->Size - Hash256::Size);
		--pMessage1->HashesCount;

		// Act:
		auto pProof = CreateFinalizationProof(statistics, { pMessage1, pMessage2 });

		// Assert:
		ASSERT_EQ(sizeof(FinalizationProof) + CalculateMessageGroupsSize(2, 5, 2), pProof->Size);
		AssertEqualStatistics(statistics, *pProof);

		auto messageGroups = pProof->MessageGroups();
		auto messageGroupsIter = messageGroups.cbegin();
		AssertMessageGroup(
				*messageGroupsIter,
				FinalizationStage::Prevote,
				Height(11),
				{ pMessage1->HashesPtr()[0], pMessage1->HashesPtr()[1] },
				{ pMessage1->Signature },
				"group 1");

		// - in the second group, only last hash differs (use pMessage1->HashesPtr for emphasis)
		++messageGroupsIter;
		AssertMessageGroup(
				*messageGroupsIter,
				FinalizationStage::Prevote,
				Height(11),
				{ pMessage1->HashesPtr()[0], pMessage1->HashesPtr()[1], pMessage2->HashesPtr()[2] },
				{ pMessage2->Signature },
				"group 2");
	}

	// endregion

	// region multiple messages

	TEST(TEST_CLASS, CanFilterFinalizationProofMessagesWithDifferentPoints) {
		// Arrange:
		auto statistics = FinalizationStatistics{ FinalizationPoint(9), Height(111), test::GenerateRandomByteArray<Hash256>() };

		auto pMessage1 = utils::UniqueToShared(test::CreateMessage(Height(11), 3));
		pMessage1->StepIdentifier = { FinalizationPoint(8), FinalizationStage::Prevote };

		auto pMessage2 = utils::UniqueToShared(test::CopyEntity(*pMessage1));
		test::FillWithRandomData(pMessage2->Signature);
		pMessage2->StepIdentifier.Point = FinalizationPoint(9);

		auto pMessage3 = utils::UniqueToShared(test::CopyEntity(*pMessage1));
		test::FillWithRandomData(pMessage3->Signature);
		pMessage3->StepIdentifier.Point = FinalizationPoint(10);

		// Act:
		auto pProof = CreateFinalizationProof(statistics, { pMessage1, pMessage2, pMessage3 });

		// Assert:
		ASSERT_EQ(sizeof(FinalizationProof) + CalculateMessageGroupsSize(1, 3, 1), pProof->Size);
		AssertEqualStatistics(statistics, *pProof);

		auto messageGroups = pProof->MessageGroups();
		auto messageGroupsIter = messageGroups.cbegin();
		AssertMessageGroup(
				*messageGroupsIter,
				FinalizationStage::Prevote,
				Height(11),
				{ pMessage1->HashesPtr()[0], pMessage1->HashesPtr()[1], pMessage1->HashesPtr()[2] },
				{ pMessage2->Signature },
				"group 1");
	}

	TEST(TEST_CLASS, CanCreateFinalizationProofWithMultipleMessages) {
		// Arrange:
		auto statistics = FinalizationStatistics{ FinalizationPoint(9), Height(111), test::GenerateRandomByteArray<Hash256>() };

		auto pMessage1 = utils::UniqueToShared(test::CreateMessage(Height(11), 3));
		pMessage1->StepIdentifier = { FinalizationPoint(9), FinalizationStage::Prevote };

		auto pMessage2 = utils::UniqueToShared(test::CopyEntity(*pMessage1));
		test::FillWithRandomData(pMessage2->Signature);

		auto pMessage3 = utils::UniqueToShared(test::CopyEntity(*pMessage1));
		test::FillWithRandomData(pMessage3->Signature);
		pMessage3->StepIdentifier.Stage = FinalizationStage::Precommit;

		auto pMessage4 = utils::UniqueToShared(test::CopyEntity(*pMessage1));
		test::FillWithRandomData(pMessage4->Signature);

		auto pMessage5 = utils::UniqueToShared(test::CopyEntity(*pMessage1));
		test::FillWithRandomData(pMessage5->Signature);
		pMessage5->StepIdentifier.Stage = FinalizationStage::Precommit;

		// Act:
		auto pProof = CreateFinalizationProof(statistics, { pMessage1, pMessage2, pMessage3, pMessage4, pMessage5 });

		// Assert:
		ASSERT_EQ(sizeof(FinalizationProof) + CalculateMessageGroupsSize(2, 6, 5), pProof->Size);
		AssertEqualStatistics(statistics, *pProof);

		auto messageGroups = pProof->MessageGroups();
		auto messageGroupsIter = messageGroups.cbegin();
		AssertMessageGroup(
				*messageGroupsIter,
				FinalizationStage::Prevote,
				Height(11),
				{ pMessage1->HashesPtr()[0], pMessage1->HashesPtr()[1], pMessage1->HashesPtr()[2] },
				{ pMessage1->Signature, pMessage2->Signature, pMessage4->Signature },
				"group 1");

		// - in the second group, only stage differs (use pMessage1->HashesPtr for emphasis)
		++messageGroupsIter;
		AssertMessageGroup(
				*messageGroupsIter,
				FinalizationStage::Precommit,
				Height(11),
				{ pMessage1->HashesPtr()[0], pMessage1->HashesPtr()[1], pMessage1->HashesPtr()[2] },
				{ pMessage3->Signature, pMessage5->Signature },
				"group 2");
	}

	// endregion
}}

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

#include "finalization/src/model/FinalizationProofUtils.h"
#include "finalization/tests/test/FinalizationMessageTestUtils.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS FinalizationProofUtilsTests

	namespace {
		// region test utils

		constexpr auto V1_Signature_Adjustment = sizeof(crypto::BmTreeSignatureV1) - sizeof(crypto::BmTreeSignature);

		FinalizationStatistics CreateFinalizationStatistics(uint32_t epoch, uint32_t point, uint64_t height) {
			return { { FinalizationEpoch(epoch), FinalizationPoint(point) }, Height(height), test::GenerateRandomByteArray<Hash256>() };
		}

		size_t CalculateMessageGroupsSize(size_t numMessageGroups, size_t numHashes, size_t numSignatures) {
			return numMessageGroups * sizeof(FinalizationMessageGroup)
					+ numHashes * Hash256::Size
					+ numSignatures * sizeof(crypto::BmTreeSignature);
		}

		void AssertEqualStatistics(const FinalizationStatistics& expectedStatistics, const FinalizationProof& proof) {
			EXPECT_EQ(expectedStatistics.Round, proof.Round);
			EXPECT_EQ(expectedStatistics.Height, proof.Height);
			EXPECT_EQ(expectedStatistics.Hash, proof.Hash);

			EXPECT_EQ(1u, proof.Version);
		}

		void AssertMessageGroup(
				const FinalizationMessageGroup& messageGroup,
				FinalizationStage expectedStage,
				Height expectedHeight,
				const std::vector<Hash256>& expectedHashes,
				const std::vector<crypto::BmTreeSignature>& expectedSignatures,
				const std::string& message) {
			ASSERT_EQ(CalculateMessageGroupsSize(1, expectedHashes.size(), expectedSignatures.size()), messageGroup.Size) << message;
			ASSERT_EQ(expectedHashes.size(), messageGroup.HashesCount) << message;
			ASSERT_EQ(expectedSignatures.size(), messageGroup.SignaturesCount) << message;

			EXPECT_EQ(1u, messageGroup.SignatureScheme) << message;
			EXPECT_EQ(expectedStage, messageGroup.Stage) << message;
			EXPECT_EQ(expectedHeight, messageGroup.Height) << message;

			for (auto i = 0u; i < expectedHashes.size(); ++i)
				EXPECT_EQ(expectedHashes[i], messageGroup.HashesPtr()[i]) << message << " hash at " << i;

			for (auto i = 0u; i < expectedSignatures.size(); ++i)
				EXPECT_EQ(expectedSignatures[i], messageGroup.SignaturesPtr()[i]) << message << " signature at " << i;
		}

		void AssertMessageGroup(
				const FinalizationMessageGroup& messageGroup,
				FinalizationStage expectedStage,
				Height expectedHeight,
				const std::vector<Hash256>& expectedHashes,
				const std::vector<crypto::BmTreeSignatureV1>& expectedSignatures,
				const std::string& message) {
			auto expectedSize =
					CalculateMessageGroupsSize(1, expectedHashes.size(), expectedSignatures.size())
					+ V1_Signature_Adjustment * expectedSignatures.size();
			ASSERT_EQ(expectedSize, messageGroup.Size) << message;
			ASSERT_EQ(expectedHashes.size(), messageGroup.HashesCount) << message;
			ASSERT_EQ(expectedSignatures.size(), messageGroup.SignaturesCount) << message;

			EXPECT_EQ(0u, messageGroup.SignatureScheme) << message;
			EXPECT_EQ(expectedStage, messageGroup.Stage) << message;
			EXPECT_EQ(expectedHeight, messageGroup.Height) << message;

			for (auto i = 0u; i < expectedHashes.size(); ++i)
				EXPECT_EQ(expectedHashes[i], messageGroup.HashesPtr()[i]) << message << " hash at " << i;

			for (auto i = 0u; i < expectedSignatures.size(); ++i)
				EXPECT_EQ(expectedSignatures[i], messageGroup.SignaturesV1Ptr()[i]) << message << " signature at " << i;
		}

		// endregion
	}

	// region zero and one message

	TEST(TEST_CLASS, CanCreateFinalizationProofWithNoMessages) {
		// Arrange:
		auto statistics = CreateFinalizationStatistics(3, 9, 111);

		// Act:
		auto pProof = CreateFinalizationProof(statistics, {});

		// Assert:
		ASSERT_EQ(sizeof(FinalizationProof), pProof->Size);
		AssertEqualStatistics(statistics, *pProof);
	}

	TEST(TEST_CLASS, CanCreateFinalizationProofWithSingleMessage) {
		// Arrange:
		auto statistics = CreateFinalizationStatistics(3, 9, 111);

		auto pMessage = utils::UniqueToShared(test::CreateMessage(Height(11), 3));
		pMessage->Data().StepIdentifier = { FinalizationEpoch(3), FinalizationPoint(9), FinalizationStage::Prevote };

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
				{ pMessage->Signature() },
				"group 1");
	}

	// endregion

	// region two messages, reducible

	TEST(TEST_CLASS, CanCreateFinalizationProofWithMultipleMessages_DifferentSignature) {
		// Arrange:
		auto statistics = CreateFinalizationStatistics(3, 9, 111);

		auto pMessage1 = utils::UniqueToShared(test::CreateMessage(Height(11), 3));
		pMessage1->Data().StepIdentifier = { FinalizationEpoch(3), FinalizationPoint(9), FinalizationStage::Prevote };

		auto pMessage2 = utils::UniqueToShared(test::CopyEntity(*pMessage1));
		test::FillWithRandomData(pMessage2->Signature());

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
				{ pMessage1->Signature(), pMessage2->Signature() },
				"group 1");
	}

	// endregion

	// region two messages, irreducible

	namespace {
		void SetPoint(StepIdentifier& stepIdentifier, FinalizationPoint point) {
			stepIdentifier = StepIdentifier(stepIdentifier.Epoch, point, stepIdentifier.Stage());
		}

		void SetStage(StepIdentifier& stepIdentifier, FinalizationStage stage) {
			stepIdentifier = StepIdentifier(stepIdentifier.Epoch, stepIdentifier.Round().Point, stage);
		}
	}

	TEST(TEST_CLASS, CanCreateFinalizationProofWithMultipleMessages_DifferentStage) {
		// Arrange:
		auto statistics = CreateFinalizationStatistics(3, 9, 111);

		auto pMessage1 = utils::UniqueToShared(test::CreateMessage(Height(11), 3));
		pMessage1->Data().StepIdentifier = { FinalizationEpoch(3), FinalizationPoint(9), FinalizationStage::Prevote };

		auto pMessage2 = utils::UniqueToShared(test::CopyEntity(*pMessage1));
		SetStage(pMessage2->Data().StepIdentifier, FinalizationStage::Precommit);

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
				{ pMessage1->Signature() },
				"group 1");

		// - in the second group, only stage differs (use pMessage1->HashesPtr for emphasis)
		++messageGroupsIter;
		AssertMessageGroup(
				*messageGroupsIter,
				FinalizationStage::Precommit,
				Height(11),
				{ pMessage1->HashesPtr()[0], pMessage1->HashesPtr()[1], pMessage1->HashesPtr()[2] },
				{ pMessage2->Signature() },
				"group 2");
	}

	TEST(TEST_CLASS, CanCreateFinalizationProofWithMultipleMessages_DifferentHeight) {
		// Arrange:
		auto statistics = CreateFinalizationStatistics(3, 9, 111);

		auto pMessage1 = utils::UniqueToShared(test::CreateMessage(Height(11), 3));
		pMessage1->Data().StepIdentifier = { FinalizationEpoch(3), FinalizationPoint(9), FinalizationStage::Prevote };

		auto pMessage2 = utils::UniqueToShared(test::CopyEntity(*pMessage1));
		pMessage2->Data().Height = Height(22);

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
				{ pMessage1->Signature() },
				"group 1");

		// - in the second group, only height differs (use pMessage1->HashesPtr for emphasis)
		++messageGroupsIter;
		AssertMessageGroup(
				*messageGroupsIter,
				FinalizationStage::Prevote,
				Height(22),
				{ pMessage1->HashesPtr()[0], pMessage1->HashesPtr()[1], pMessage1->HashesPtr()[2] },
				{ pMessage2->Signature() },
				"group 2");
	}

	TEST(TEST_CLASS, CanCreateFinalizationProofWithMultipleMessages_DifferentHashesContents) {
		// Arrange:
		auto statistics = CreateFinalizationStatistics(3, 9, 111);

		auto pMessage1 = utils::UniqueToShared(test::CreateMessage(Height(11), 3));
		pMessage1->Data().StepIdentifier = { FinalizationEpoch(3), FinalizationPoint(9), FinalizationStage::Prevote };
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
				{ pMessage1->Signature() },
				"group 1");

		// - in the second group, only last hash differs (use pMessage1->HashesPtr for emphasis)
		++messageGroupsIter;
		AssertMessageGroup(
				*messageGroupsIter,
				FinalizationStage::Prevote,
				Height(11),
				{ pMessage1->HashesPtr()[0], pMessage1->HashesPtr()[1], pMessage2->HashesPtr()[2] },
				{ pMessage2->Signature() },
				"group 2");
	}

	TEST(TEST_CLASS, CanCreateFinalizationProofWithMultipleMessages_DifferentHashesNumber) {
		// Arrange:
		auto statistics = CreateFinalizationStatistics(3, 9, 111);

		auto pMessage1 = utils::UniqueToShared(test::CreateMessage(Height(11), 3));
		pMessage1->Data().StepIdentifier = { FinalizationEpoch(3), FinalizationPoint(9), FinalizationStage::Prevote };

		auto pMessage2 = utils::UniqueToShared(test::CopyEntity(*pMessage1));
		pMessage1->Size = static_cast<uint32_t>(pMessage1->Size - Hash256::Size);
		--pMessage1->Data().HashesCount;

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
				{ pMessage1->Signature() },
				"group 1");

		// - in the second group, only last hash differs (use pMessage1->HashesPtr for emphasis)
		++messageGroupsIter;
		AssertMessageGroup(
				*messageGroupsIter,
				FinalizationStage::Prevote,
				Height(11),
				{ pMessage1->HashesPtr()[0], pMessage1->HashesPtr()[1], pMessage2->HashesPtr()[2] },
				{ pMessage2->Signature() },
				"group 2");
	}

	TEST(TEST_CLASS, CanCreateFinalizationProofWithMultipleMessages_DifferentSignatureSchemes) {
		// Arrange:
		auto statistics = CreateFinalizationStatistics(3, 9, 111);

		auto pMessage1 = utils::UniqueToShared(test::CreateMessageV1(Height(11), 3));
		pMessage1->Data().StepIdentifier = { FinalizationEpoch(3), FinalizationPoint(9), FinalizationStage::Prevote };

		auto pMessage2 = utils::UniqueToShared(test::CreateMessage(Height(11), 3));
		pMessage2->Data().StepIdentifier = { FinalizationEpoch(3), FinalizationPoint(9), FinalizationStage::Prevote };
		std::memcpy(reinterpret_cast<void*>(pMessage2->HashesPtr()), pMessage1->HashesPtr(), 3 * Hash256::Size);

		// Act:
		auto pProof = CreateFinalizationProof(statistics, { pMessage1, pMessage2 });

		// Assert:
		ASSERT_EQ(sizeof(FinalizationProof) + CalculateMessageGroupsSize(2, 6, 2) + V1_Signature_Adjustment, pProof->Size);
		AssertEqualStatistics(statistics, *pProof);

		auto messageGroups = pProof->MessageGroups();
		auto messageGroupsIter = messageGroups.cbegin();
		AssertMessageGroup(
				*messageGroupsIter,
				FinalizationStage::Prevote,
				Height(11),
				{ pMessage1->HashesPtr()[0], pMessage1->HashesPtr()[1], pMessage1->HashesPtr()[2] },
				{ pMessage1->SignatureV1() },
				"group 1");

		// - in the second group, only signature scheme differs (use pMessage1->HashesPtr for emphasis)
		++messageGroupsIter;
		AssertMessageGroup(
				*messageGroupsIter,
				FinalizationStage::Prevote,
				Height(11),
				{ pMessage1->HashesPtr()[0], pMessage1->HashesPtr()[1], pMessage1->HashesPtr()[2] },
				{ pMessage2->Signature() },
				"group 2");
	}

	// endregion

	// region multiple messages - filtering

	namespace {
		void RunFilteredMessageTest(const consumer<FinalizationMessage&, uint32_t>& messageModifier) {
			// Arrange:
			auto statistics = CreateFinalizationStatistics(3, 9, 111);

			auto pMessage1 = utils::UniqueToShared(test::CreateMessage(Height(11), 3));
			pMessage1->Data().StepIdentifier = { FinalizationEpoch(3), FinalizationPoint(9), FinalizationStage::Prevote };
			messageModifier(*pMessage1, 0);

			auto pMessage2 = utils::UniqueToShared(test::CopyEntity(*pMessage1));
			test::FillWithRandomData(pMessage2->Signature());
			messageModifier(*pMessage2, 1);

			auto pMessage3 = utils::UniqueToShared(test::CopyEntity(*pMessage1));
			test::FillWithRandomData(pMessage3->Signature());
			messageModifier(*pMessage3, 2);

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
					{ pMessage2->Signature() },
					"group 1");
		}
	}

	TEST(TEST_CLASS, CanFilterFinalizationProofMessagesWithDifferentEpochs) {
		RunFilteredMessageTest([](auto& message, auto id) {
			message.Data().StepIdentifier.Epoch = FinalizationEpoch(2 + id);
		});
	}

	TEST(TEST_CLASS, CanFilterFinalizationProofMessagesWithDifferentPoints) {
		RunFilteredMessageTest([](auto& message, auto id) {
			SetPoint(message.Data().StepIdentifier, FinalizationPoint(8 + id));
		});
	}

	TEST(TEST_CLASS, CanFilterFinalizationProofMessagesWithDifferentVersions) {
		RunFilteredMessageTest([](auto& message, auto id) {
			message.Data().Version = id;
		});
	}

	TEST(TEST_CLASS, CanFilterFinalizationProofMessagesWithInvalidPadding) {
		RunFilteredMessageTest([](auto& message, auto id) {
			message.FinalizationMessage_Reserved1 = 0 == id % 2 ? 1 : 0;
		});
	}

	// endregion

	// region multiple messages - success

	TEST(TEST_CLASS, CanCreateFinalizationProofWithMultipleMessages) {
		// Arrange:
		auto statistics = CreateFinalizationStatistics(3, 9, 111);

		auto pMessage1 = utils::UniqueToShared(test::CreateMessage(Height(11), 3));
		pMessage1->Data().StepIdentifier = { FinalizationEpoch(3), FinalizationPoint(9), FinalizationStage::Prevote };

		auto pMessage2 = utils::UniqueToShared(test::CopyEntity(*pMessage1));
		test::FillWithRandomData(pMessage2->Signature());

		auto pMessage3 = utils::UniqueToShared(test::CopyEntity(*pMessage1));
		test::FillWithRandomData(pMessage3->Signature());
		SetStage(pMessage3->Data().StepIdentifier, FinalizationStage::Precommit);

		auto pMessage4 = utils::UniqueToShared(test::CopyEntity(*pMessage1));
		test::FillWithRandomData(pMessage4->Signature());

		auto pMessage5 = utils::UniqueToShared(test::CopyEntity(*pMessage1));
		test::FillWithRandomData(pMessage5->Signature());
		SetStage(pMessage5->Data().StepIdentifier, FinalizationStage::Precommit);

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
				{ pMessage1->Signature(), pMessage2->Signature(), pMessage4->Signature() },
				"group 1");

		// - in the second group, only stage differs (use pMessage1->HashesPtr for emphasis)
		++messageGroupsIter;
		AssertMessageGroup(
				*messageGroupsIter,
				FinalizationStage::Precommit,
				Height(11),
				{ pMessage1->HashesPtr()[0], pMessage1->HashesPtr()[1], pMessage1->HashesPtr()[2] },
				{ pMessage3->Signature(), pMessage5->Signature() },
				"group 2");
	}

	// endregion
}}

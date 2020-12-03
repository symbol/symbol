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

#include "finalization/src/model/FinalizationMessage.h"
#include "catapult/crypto_voting/AggregateBmPrivateKeyTree.h"
#include "finalization/tests/test/FinalizationMessageTestUtils.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/core/HashTestUtils.h"
#include "tests/test/core/VariableSizedEntityTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/test/nodeps/NumericTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS FinalizationMessageTests

	namespace {
		// region test utils - message

		std::unique_ptr<FinalizationMessage> CreateMessage(uint32_t numHashes) {
			return test::CreateMessage(test::GenerateRandomValue<Height>(), numHashes);
		}

		// endregion
	}

	// region FinalizationMessage (size + alignment)

#define MESSAGE_FIELDS FIELD(Signature) FIELD(Version) FIELD(HashesCount) FIELD(StepIdentifier) FIELD(Height)

	TEST(TEST_CLASS, FinalizationMessageHasExpectedSize) {
		// Arrange:
		auto expectedSize = sizeof(TrailingVariableDataLayout<FinalizationMessage, Hash256>) + 4;

#define FIELD(X) expectedSize += sizeof(FinalizationMessage::X);
		MESSAGE_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(FinalizationMessage));
		EXPECT_EQ(4 + 4 + 216u, sizeof(FinalizationMessage));
	}

	TEST(TEST_CLASS, FinalizationMessageHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(FinalizationMessage, X);
		MESSAGE_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(FinalizationMessage) % 8);
	}

#undef MESSAGE_FIELDS

	// endregion

	// region FinalizationMessage (CalculateRealSize)

	TEST(TEST_CLASS, CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		FinalizationMessage message;
		message.Size = 0;
		message.HashesCount = 67;

		// Act:
		auto realSize = FinalizationMessage::CalculateRealSize(message);

		// Assert:
		EXPECT_EQ(sizeof(FinalizationMessage) + 67 * Hash256::Size, realSize);
	}

	TEST(TEST_CLASS, CalculateRealSizeDoesNotOverflowWithMaxValues) {
		// Arrange:
		FinalizationMessage message;
		message.Size = 0;
		test::SetMaxValue(message.HashesCount);

		// Act:
		auto realSize = FinalizationMessage::CalculateRealSize(message);

		// Assert:
		EXPECT_EQ(sizeof(FinalizationMessage) + message.HashesCount * Hash256::Size, realSize);
		EXPECT_GE(std::numeric_limits<uint64_t>::max(), realSize);
	}

	// endregion

	// region FinalizationMessage (data pointers)

	namespace {
		struct FinalizationMessageTraits {
			static constexpr auto GenerateEntityWithAttachments = CreateMessage;

			template<typename TEntity>
			static auto GetAttachmentPointer(TEntity& entity) {
				return entity.HashesPtr();
			}
		};
	}

	DEFINE_ATTACHMENT_POINTER_TESTS(TEST_CLASS, FinalizationMessageTraits) // HashesPtr

	// endregion

	// region FinalizationMessage (insertion operator)

	namespace {
		void AssertStringRepresentation(FinalizationStage stage, const std::string& expectedStageName) {
			// Arrange:
			std::vector<std::string> hashStrings{
				"D14E96D509A0966A7CCBFEE7F3D92AB2AC05C4A42DFB05B3A8C8EFD5C35B9620",
				"13C519B8A78B0AFC78E15E52EAC9B1208BB9CF961B274A9D138CD161C08F033C",
				"E5BA68896934C6F2745B528AB636761AFC9BEE8A32B3CD7E9CF4C0E16A00C633",
				"2E6BD2BBB35EA998677C5A67DAC13F7E6377FA898427ABCB14274E49812FF013"
			};

			auto pMessage = test::CreateMessage(Height(12), 3);
			pMessage->StepIdentifier = { FinalizationEpoch(101), FinalizationPoint(17), stage };
			pMessage->Signature.Root.ParentPublicKey = utils::ParseByteArray<VotingKey>(hashStrings[0]);
			for (auto i = 0u; i < pMessage->HashesCount; ++i)
				pMessage->HashesPtr()[i] = utils::ParseByteArray<Hash256>(hashStrings[i + 1]);

			// Act:
			auto str = test::ToString(*pMessage);

			// Assert:
			auto expected = "message for (101, 17) " + expectedStageName + " at 12 from " + hashStrings[0]
					+ "\n + " + hashStrings[1] + " @ 12"
					+ "\n + " + hashStrings[2] + " @ 13"
					+ "\n + " + hashStrings[3] + " @ 14";
			EXPECT_EQ(expected, str);
		}
	}

	TEST(TEST_CLASS, CanOutputPrevoteMessage) {
		AssertStringRepresentation(FinalizationStage::Prevote, "prevote");
	}

	TEST(TEST_CLASS, CanOutputPrecommitMessage) {
		AssertStringRepresentation(FinalizationStage::Precommit, "precommit");
	}

	// endregion

	// region CalculateMessageHash

	TEST(TEST_CLASS, CalculateMessageHash_ProducesDifferentHashesForMessagesWithDifferentBodyContents) {
		// Arrange:
		auto pMessage1 = CreateMessage(3);
		auto pMessage2 = test::CopyEntity(*pMessage1);
		pMessage2->StepIdentifier.Epoch = pMessage2->StepIdentifier.Epoch + FinalizationEpoch(1);

		// Act:
		auto messageHash1 = CalculateMessageHash(*pMessage1);
		auto messageHash2 = CalculateMessageHash(*pMessage2);

		// Assert:
		EXPECT_NE(messageHash1, messageHash2);
	}

	TEST(TEST_CLASS, CalculateMessageHash_ProducesDifferentHashesForMessagesWithDifferentHeaderContents) {
		// Arrange:
		auto pMessage1 = CreateMessage(3);
		auto pMessage2 = test::CopyEntity(*pMessage1);
		test::FillWithRandomData(pMessage2->Signature);

		// Act:
		auto messageHash1 = CalculateMessageHash(*pMessage1);
		auto messageHash2 = CalculateMessageHash(*pMessage2);

		// Assert:
		EXPECT_NE(messageHash1, messageHash2);
	}

	// endregion

	namespace {
		// region test utils - FinalizationContext

		enum class VoterType : uint32_t { Small, Large, Ineligible };

		constexpr auto Expected_Large_Weight = 4'000'000u;

		template<typename TAction>
		void RunFinalizationContextTest(TAction action) {
			// Arrange:
			auto config = finalization::FinalizationConfiguration::Uninitialized();
			auto finalizationContextPair = test::CreateFinalizationContext(config, FinalizationEpoch(50), Height(123), {
				Amount(2'000'000), Amount(Expected_Large_Weight), Amount(1'000'000), Amount(6'000'000)
			});

			// Act + Assert:
			action(finalizationContextPair.first, finalizationContextPair.second);
		}

		// endregion
	}

	// region PrepareMessage

	namespace {
		StepIdentifier DefaultStepIdentifier() {
			return { FinalizationEpoch(4), FinalizationPoint(3), FinalizationStage::Prevote };
		}

		template<typename TAction>
		void RunPrepareMessageTest(VoterType voterType, uint32_t numHashes, const StepIdentifier& messageStepIdentifier, TAction action) {
			// Arrange:
			RunFinalizationContextTest([voterType, numHashes, messageStepIdentifier, action](
					const auto& context,
					const auto& keyPairDescriptors) {
				const auto& keyPairDescriptor = keyPairDescriptors[utils::to_underlying_type(voterType)];

				auto numFactoryCalls = 0u;
				auto storage = mocks::MockSeekableMemoryStream();
				auto bmPrivateKeyTree = crypto::AggregateBmPrivateKeyTree([&keyPairDescriptor, &numFactoryCalls, &storage]() {
					if (++numFactoryCalls > 1)
						return std::unique_ptr<crypto::BmPrivateKeyTree>();

					auto bmOptions = crypto::BmOptions{ { 0 }, { 15 } };
					auto tree = crypto::BmPrivateKeyTree::Create(test::CopyKeyPair(keyPairDescriptor.VotingKeyPair), storage, bmOptions);
					return std::make_unique<crypto::BmPrivateKeyTree>(std::move(tree));
				});

				auto hashes = test::GenerateRandomHashes(numHashes);

				// Act:
				auto pMessage = PrepareMessage(bmPrivateKeyTree, messageStepIdentifier, Height(987), hashes);

				// Assert:
				action(pMessage, context, hashes);
			});
		}

		template<typename TAction>
		void RunPrepareMessageTest(VoterType voterType, uint32_t numHashes, TAction action) {
			RunPrepareMessageTest(voterType, numHashes, DefaultStepIdentifier(), action);
		}

		HashRange ExtractHashes(const FinalizationMessage& message) {
			return HashRange::CopyFixed(reinterpret_cast<const uint8_t*>(message.HashesPtr()), message.HashesCount);
		}
	}

	TEST(TEST_CLASS, PrepareMessage_CannotPrepareMessageWithUnsupportedStepIdentifier) {
		// Arrange:
		auto outOfBoundsStepIdentifier = DefaultStepIdentifier();
		outOfBoundsStepIdentifier.Epoch = FinalizationEpoch(20);
		RunPrepareMessageTest(VoterType::Large, 0, outOfBoundsStepIdentifier, [](const auto& pMessage, const auto&, const auto&) {
			// Assert:
			EXPECT_FALSE(!!pMessage);
		});
	}

	TEST(TEST_CLASS, PrepareMessage_CanPrepareValidMessageWithoutHashes) {
		// Arrange:
		RunPrepareMessageTest(VoterType::Large, 0, [](const auto& pMessage, const auto& context, const auto& hashes) {
			// Assert:
			ASSERT_TRUE(!!pMessage);

			// - check a few fields
			EXPECT_EQ(sizeof(FinalizationMessage), pMessage->Size);

			EXPECT_EQ(0u, pMessage->FinalizationMessage_Reserved1);
			EXPECT_EQ(FinalizationMessage::Current_Version, pMessage->Version);
			EXPECT_EQ(0u, pMessage->HashesCount);

			EXPECT_EQ(DefaultStepIdentifier(), pMessage->StepIdentifier);
			EXPECT_EQ(Height(987), pMessage->Height);
			EXPECT_EQ(0u, FindFirstDifferenceIndex(hashes, ExtractHashes(*pMessage)));

			// - check that the message is valid and can be processed
			auto processResultPair = ProcessMessage(*pMessage, context);
			EXPECT_EQ(ProcessMessageResult::Success, processResultPair.first);
			EXPECT_EQ(Expected_Large_Weight, processResultPair.second);
		});
	}

	TEST(TEST_CLASS, PrepareMessage_CanPrepareValidMessageWithHashes) {
		// Arrange:
		RunPrepareMessageTest(VoterType::Large, 3, [](const auto& pMessage, const auto& context, const auto& hashes) {
			// Assert:
			ASSERT_TRUE(!!pMessage);

			// - check a few fields
			EXPECT_EQ(sizeof(FinalizationMessage) + 3 * Hash256::Size, pMessage->Size);

			EXPECT_EQ(0u, pMessage->FinalizationMessage_Reserved1);
			EXPECT_EQ(FinalizationMessage::Current_Version, pMessage->Version);
			EXPECT_EQ(3u, pMessage->HashesCount);

			EXPECT_EQ(DefaultStepIdentifier(), pMessage->StepIdentifier);
			EXPECT_EQ(Height(987), pMessage->Height);
			EXPECT_EQ(3u, FindFirstDifferenceIndex(hashes, ExtractHashes(*pMessage)));

			// - check that the message is valid and can be processed
			auto processResultPair = ProcessMessage(*pMessage, context);
			EXPECT_EQ(ProcessMessageResult::Success, processResultPair.first);
			EXPECT_EQ(Expected_Large_Weight, processResultPair.second);
		});
	}

	TEST(TEST_CLASS, PrepareMessage_CanPrepareValidMessageWithHashesForIneligibleVoter) {
		// Arrange:
		RunPrepareMessageTest(VoterType::Ineligible, 3, [](const auto& pMessage, const auto& context, const auto& hashes) {
			// Assert:
			ASSERT_TRUE(!!pMessage);

			// - check a few fields
			EXPECT_EQ(sizeof(FinalizationMessage) + 3 * Hash256::Size, pMessage->Size);
			EXPECT_EQ(3u, pMessage->HashesCount);

			EXPECT_EQ(DefaultStepIdentifier(), pMessage->StepIdentifier);
			EXPECT_EQ(Height(987), pMessage->Height);
			EXPECT_EQ(3u, FindFirstDifferenceIndex(hashes, ExtractHashes(*pMessage)));

			// - check that the message signer is not a valid voter
			auto processResultPair = ProcessMessage(*pMessage, context);
			EXPECT_EQ(ProcessMessageResult::Failure_Voter, processResultPair.first);
			EXPECT_EQ(0u, processResultPair.second);
		});
	}

	// endregion

	// region ProcessMessage

	namespace {
		template<typename TAction>
		void RunProcessMessageTest(
				VoterType voterType,
				uint32_t numHashes,
				const consumer<FinalizationMessage&>& modifyMessage,
				TAction action) {
			// Arrange:
			RunFinalizationContextTest([voterType, numHashes, modifyMessage, action](const auto& context, const auto& keyPairDescriptors) {
				const auto& keyPairDescriptor = keyPairDescriptors[utils::to_underlying_type(voterType)];

				// - create message
				auto pMessage = CreateMessage(numHashes);
				pMessage->FinalizationMessage_Reserved1 = 0;
				pMessage->Version = FinalizationMessage::Current_Version;
				pMessage->StepIdentifier = DefaultStepIdentifier();
				pMessage->Height = Height(987);
				modifyMessage(*pMessage);
				test::SignMessage(*pMessage, keyPairDescriptor.VotingKeyPair);

				// Act + Assert:
				action(context, keyPairDescriptor, *pMessage);
			});
		}

		template<typename TAction>
		void RunProcessMessageTest(VoterType voterType, uint32_t numHashes, TAction action) {
			RunProcessMessageTest(voterType, numHashes, [](const auto&){}, action);
		}
	}

	TEST(TEST_CLASS, ProcessMessage_FailsWhenSignatureIsInvalid) {
		// Arrange:
		RunProcessMessageTest(VoterType::Large, 3, [](const auto& context, const auto&, auto& message) {
			// - corrupt a hash
			test::FillWithRandomData(message.HashesPtr()[1]);

			// Act:
			auto processResultPair = ProcessMessage(message, context);

			// Assert:
			EXPECT_EQ(ProcessMessageResult::Failure_Signature, processResultPair.first);
			EXPECT_EQ(0u, processResultPair.second);
		});
	}

	TEST(TEST_CLASS, ProcessMessage_FailsWhenReservedDataIsNotCleared) {
		// Arrange:
		auto modifyMessage = [](auto& message) { message.FinalizationMessage_Reserved1 = 1; };
		RunProcessMessageTest(VoterType::Large, 3, modifyMessage, [](const auto& context, const auto&, const auto& message) {
			// Act:
			auto processResultPair = ProcessMessage(message, context);

			// Assert:
			EXPECT_EQ(ProcessMessageResult::Failure_Padding, processResultPair.first);
			EXPECT_EQ(0u, processResultPair.second);
		});
	}

	TEST(TEST_CLASS, ProcessMessage_FailsWhenVersionIsIncorrect) {
		// Arrange:
		auto modifyMessage = [](auto& message) { ++message.Version; };
		RunProcessMessageTest(VoterType::Large, 3, modifyMessage, [](const auto& context, const auto&, const auto& message) {
			// Act:
			auto processResultPair = ProcessMessage(message, context);

			// Assert:
			EXPECT_EQ(ProcessMessageResult::Failure_Version, processResultPair.first);
			EXPECT_EQ(0u, processResultPair.second);
		});
	}

	TEST(TEST_CLASS, ProcessMessage_FailsWhenAccountIsNotVotingEligible) {
		// Arrange:
		RunProcessMessageTest(VoterType::Ineligible, 3, [](const auto& context, const auto&, const auto& message) {
			// Act:
			auto processResultPair = ProcessMessage(message, context);

			// Assert:
			EXPECT_EQ(ProcessMessageResult::Failure_Voter, processResultPair.first);
			EXPECT_EQ(0u, processResultPair.second);
		});
	}

	TEST(TEST_CLASS, ProcessMessage_CanProcessValidMessageWithoutHashes) {
		// Arrange:
		RunProcessMessageTest(VoterType::Large, 0, [](const auto& context, const auto&, const auto& message) {
			// Act:
			auto processResultPair = ProcessMessage(message, context);

			// Assert:
			EXPECT_EQ(ProcessMessageResult::Success, processResultPair.first);
			EXPECT_EQ(Expected_Large_Weight, processResultPair.second);

			// Sanity:
			EXPECT_EQ(0u, message.HashesCount);
		});
	}

	TEST(TEST_CLASS, ProcessMessage_CanProcessValidMessageWithHashes) {
		// Arrange:
		RunProcessMessageTest(VoterType::Large, 3, [](const auto& context, const auto&, const auto& message) {
			// Act:
			auto processResultPair = ProcessMessage(message, context);

			// Assert:
			EXPECT_EQ(ProcessMessageResult::Success, processResultPair.first);
			EXPECT_EQ(Expected_Large_Weight, processResultPair.second);

			// Sanity:
			EXPECT_EQ(3u, message.HashesCount);
		});
	}

	// endregion
}}

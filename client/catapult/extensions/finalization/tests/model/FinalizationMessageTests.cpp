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

#include "finalization/src/model/FinalizationMessage.h"
#include "catapult/crypto_voting/OtsTree.h"
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

#define MESSAGE_FIELDS FIELD(HashesCount) FIELD(Signature) FIELD(StepIdentifier) FIELD(Height)

	TEST(TEST_CLASS, FinalizationMessageHasExpectedSize) {
		// Arrange:
		auto expectedSize = sizeof(TrailingVariableDataLayout<FinalizationMessage, Hash256>);

#define FIELD(X) expectedSize += sizeof(FinalizationMessage::X);
		MESSAGE_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(FinalizationMessage));
		EXPECT_EQ(4 + 316u, sizeof(FinalizationMessage));
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

	// region CalculateMessageHash

	TEST(TEST_CLASS, CalculateMessageHash_ProducesDifferentHashesForMessagesWithDifferentBodyContents) {
		// Arrange:
		auto pMessage1 = CreateMessage(3);
		auto pMessage2 = test::CopyEntity(*pMessage1);
		pMessage2->StepIdentifier.Point = pMessage2->StepIdentifier.Point + FinalizationPoint(1);

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
			config.OtsKeyDilution = 13u;

			auto finalizationContextPair = test::CreateFinalizationContext(config, FinalizationPoint(50), Height(123), {
				Amount(2'000'000), Amount(Expected_Large_Weight), Amount(1'000'000), Amount(6'000'000)
			});

			// Act + Assert:
			action(finalizationContextPair.first, finalizationContextPair.second);
		}

		// endregion
	}

	// region IsEligibleVoter

	namespace {
		void RunIsEligibleVoterTest(bool expectedResult, VoterType voterType) {
			// Arrange:
			RunFinalizationContextTest([expectedResult, voterType](const auto& context, const auto& keyPairDescriptors) {
				const auto& keyPairDescriptor = keyPairDescriptors[utils::to_underlying_type(voterType)];

				auto storage = mocks::MockSeekableMemoryStream();
				auto otsTree = crypto::OtsTree::Create(
						test::CopyKeyPair(keyPairDescriptor.VotingKeyPair),
						storage,
						{ context.config().OtsKeyDilution, { 0, 2 }, { 15, 1 } });

				// Act:
				auto isEligibleVoter = IsEligibleVoter(otsTree, context);

				// Assert:
				EXPECT_EQ(expectedResult, isEligibleVoter);
			});
		}
	}

	TEST(TEST_CLASS, IsEligibleVoterReturnsFalseWhenVoterIsNotEligible) {
		RunIsEligibleVoterTest(false, VoterType::Ineligible);
	}

	TEST(TEST_CLASS, IsEligibleVoterReturnsTrueWhenVoterIsEligible) {
		RunIsEligibleVoterTest(true, VoterType::Large);
	}

	// endregion

	// region PrepareMessage

	namespace {
		constexpr auto Default_Step_Identifier = StepIdentifier{ FinalizationPoint(3), FinalizationStage::Prevote };

		template<typename TAction>
		void RunPrepareMessageTest(VoterType voterType, uint32_t numHashes, TAction action) {
			// Arrange:
			RunFinalizationContextTest([voterType, numHashes, action](const auto& context, const auto& keyPairDescriptors) {
				const auto& keyPairDescriptor = keyPairDescriptors[utils::to_underlying_type(voterType)];

				auto storage = mocks::MockSeekableMemoryStream();
				auto otsTree = crypto::OtsTree::Create(
						test::CopyKeyPair(keyPairDescriptor.VotingKeyPair),
						storage,
						{ context.config().OtsKeyDilution, { 0, 2 }, { 15, 1 } });

				auto hashes = test::GenerateRandomHashes(numHashes);

				// Act:
				auto pMessage = PrepareMessage(otsTree, Default_Step_Identifier, Height(987), hashes);

				// Assert:
				action(pMessage, context, hashes);
			});
		}

		HashRange ExtractHashes(const FinalizationMessage& message) {
			return HashRange::CopyFixed(reinterpret_cast<const uint8_t*>(message.HashesPtr()), message.HashesCount);
		}
	}

	TEST(TEST_CLASS, PrepareMessage_CanPrepareValidMessageWithoutHashes) {
		// Arrange:
		RunPrepareMessageTest(VoterType::Large, 0, [](const auto& pMessage, const auto& context, const auto& hashes) {
			// Assert:
			ASSERT_TRUE(!!pMessage);

			// - check a few fields
			EXPECT_EQ(sizeof(FinalizationMessage), pMessage->Size);
			EXPECT_EQ(0u, pMessage->HashesCount);

			EXPECT_EQ(Default_Step_Identifier, pMessage->StepIdentifier);
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
			EXPECT_EQ(3u, pMessage->HashesCount);

			EXPECT_EQ(Default_Step_Identifier, pMessage->StepIdentifier);
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

			EXPECT_EQ(Default_Step_Identifier, pMessage->StepIdentifier);
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
		void RunProcessMessageTest(VoterType voterType, const StepIdentifier& stepIdentifier, uint32_t numHashes, TAction action) {
			// Arrange:
			RunFinalizationContextTest([&stepIdentifier, voterType, numHashes, action](
					const auto& context,
					const auto& keyPairDescriptors) {
				const auto& keyPairDescriptor = keyPairDescriptors[utils::to_underlying_type(voterType)];

				// - create message
				auto pMessage = CreateMessage(numHashes);
				pMessage->StepIdentifier = stepIdentifier;
				pMessage->Height = Height(987);
				test::SignMessage(*pMessage, keyPairDescriptor.VotingKeyPair);

				// Act + Assert:
				action(context, keyPairDescriptor, *pMessage);
			});
		}

		template<typename TAction>
		void RunProcessMessageTest(VoterType voterType, uint32_t numHashes, TAction action) {
			RunProcessMessageTest(voterType, Default_Step_Identifier, numHashes, action);
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
			EXPECT_EQ(ProcessMessageResult::Failure_Message_Signature, processResultPair.first);
			EXPECT_EQ(0u, processResultPair.second);
		});
	}

	TEST(TEST_CLASS, ProcessMessage_FailsWhenStageIsInvalid) {
		// Arrange:
		for (auto stage : std::initializer_list<uint64_t>{ 2, 10 }) {
			auto stepIdentifier = Default_Step_Identifier;
			stepIdentifier.Stage = static_cast<FinalizationStage>(stage);
			RunProcessMessageTest(VoterType::Large, stepIdentifier, 3, [](const auto& context, const auto&, const auto& message) {
				// Act:
				auto processResultPair = ProcessMessage(message, context);

				// Assert:
				EXPECT_EQ(ProcessMessageResult::Failure_Stage, processResultPair.first);
				EXPECT_EQ(0u, processResultPair.second);
			});
		}
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

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
#define TEST_CLASS_V1 FinalizationMessageTests_V1

	// region traits

	namespace {
		struct CurrentTraits {
			using TreeSignature = crypto::BmTreeSignature;

			static constexpr uint16_t Signature_Scheme = 1;
			static constexpr uint32_t Base_Size = SizeOf32<FinalizationMessage>()
					+ SizeOf32<TreeSignature>()
					+ SizeOf32<FinalizationMessagePayload>();

			static std::unique_ptr<FinalizationMessage> CreateMessage(uint32_t numHashes) {
				return test::CreateMessage(test::GenerateRandomValue<Height>(), numHashes);
			}
		};

		struct V1Traits {
			using TreeSignature = crypto::BmTreeSignatureV1;

			static constexpr uint16_t Signature_Scheme = 0;
			static constexpr uint32_t Base_Size = SizeOf32<FinalizationMessage>()
					+ SizeOf32<TreeSignature>()
					+ SizeOf32<FinalizationMessagePayload>();

			static std::unique_ptr<FinalizationMessage> CreateMessage(uint32_t numHashes) {
				return test::CreateMessageV1(test::GenerateRandomValue<Height>(), numHashes);
			}
		};
	}

#define VERSION_TRAITS(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CurrentTraits>(); } \
	TEST(TEST_CLASS_V1, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<V1Traits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region FinalizationMessage (size + alignment)

#define MESSAGE_FIELDS FIELD(SignatureScheme)

	TEST(TEST_CLASS, FinalizationMessageHasExpectedSize) {
		// Arrange:
		auto expectedSize = sizeof(TrailingVariableDataLayout<FinalizationMessage, Hash256>) + 2;

#define FIELD(X) expectedSize += sizeof(FinalizationMessage::X);
		MESSAGE_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(FinalizationMessage));
		EXPECT_EQ(4 + 2 + 2u, sizeof(FinalizationMessage));
	}

	TEST(TEST_CLASS, FinalizationMessageHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(FinalizationMessage, X);
		MESSAGE_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(FinalizationMessage) % 8);
	}

#undef MESSAGE_FIELDS

	// endregion

	// region FinalizationMessagePayload (size + alignment)

#define MESSAGE_PAYLOAD_FIELDS FIELD(Version) FIELD(HashesCount) FIELD(StepIdentifier) FIELD(Height)

	TEST(TEST_CLASS, FinalizationMessagePayloadHasExpectedSize) {
		// Arrange:
		auto expectedSize = 0u;

#define FIELD(X) expectedSize += SizeOf32<decltype(FinalizationMessagePayload::X)>();
		MESSAGE_PAYLOAD_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(FinalizationMessagePayload));
		EXPECT_EQ(24u, sizeof(FinalizationMessagePayload));
	}

	TEST(TEST_CLASS, FinalizationMessagePayloadHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(FinalizationMessagePayload, X);
		MESSAGE_PAYLOAD_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(FinalizationMessagePayload) % 8);
	}

#undef MESSAGE_PAYLOAD_FIELDS

	// endregion

	// region FinalizationMessage (CalculateRealSize)

	VERSION_TRAITS(CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		auto pMessage = TTraits::CreateMessage(0);
		pMessage->Size = 0;
		pMessage->SignatureScheme = TTraits::Signature_Scheme;
		pMessage->Data().HashesCount = 67;

		// Act:
		auto realSize = FinalizationMessage::CalculateRealSize(*pMessage);

		// Assert:
		EXPECT_EQ(TTraits::Base_Size + 67 * Hash256::Size, realSize);
	}

	VERSION_TRAITS(CalculateRealSizeDoesNotOverflowWithMaxValues) {
		// Arrange:
		auto pMessage = TTraits::CreateMessage(0);
		pMessage->Size = 0;
		pMessage->SignatureScheme = TTraits::Signature_Scheme;
		test::SetMaxValue(pMessage->Data().HashesCount);

		// Act:
		auto realSize = FinalizationMessage::CalculateRealSize(*pMessage);

		// Assert:
		EXPECT_EQ(TTraits::Base_Size + pMessage->Data().HashesCount * Hash256::Size, realSize);
		EXPECT_GE(std::numeric_limits<uint64_t>::max(), realSize);
	}

	// endregion

	// region FinalizationMessage (data pointers)

	namespace {
		template<typename TTraits>
		struct FinalizationMessageTraits {
			static constexpr auto GenerateEntityWithAttachments = TTraits::CreateMessage;

			static constexpr auto Offset = SizeOf32<typename TTraits::TreeSignature>() + SizeOf32<FinalizationMessagePayload>();

			template<typename TEntity>
			static auto GetAttachmentPointer(TEntity& entity) {
				return entity.HashesPtr();
			}
		};
	}

	DEFINE_ATTACHMENT_POINTER_TESTS(TEST_CLASS, FinalizationMessageTraits<CurrentTraits>) // HashesPtr
	DEFINE_ATTACHMENT_POINTER_TESTS(TEST_CLASS_V1, FinalizationMessageTraits<V1Traits>) // HashesPtr

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
			pMessage->Data().StepIdentifier = { FinalizationEpoch(101), FinalizationPoint(17), stage };
			pMessage->Signature().Root.ParentPublicKey = utils::ParseByteArray<VotingKey>(hashStrings[0]);
			for (auto i = 0u; i < pMessage->Data().HashesCount; ++i)
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

	VERSION_TRAITS(CalculateMessageHash_ProducesDifferentHashesForMessagesWithDifferentBodyContents) {
		// Arrange:
		auto pMessage1 = TTraits::CreateMessage(3);
		auto pMessage2 = test::CopyEntity(*pMessage1);
		pMessage2->Data().StepIdentifier.Epoch = pMessage2->Data().StepIdentifier.Epoch + FinalizationEpoch(1);

		// Act:
		auto messageHash1 = CalculateMessageHash(*pMessage1);
		auto messageHash2 = CalculateMessageHash(*pMessage2);

		// Assert:
		EXPECT_NE(messageHash1, messageHash2);
	}

	VERSION_TRAITS(CalculateMessageHash_ProducesDifferentHashesForMessagesWithDifferentHeaderContents) {
		// Arrange:
		auto pMessage1 = TTraits::CreateMessage(3);
		auto pMessage2 = test::CopyEntity(*pMessage1);
		test::FillWithRandomData(pMessage2->Signature());

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
			return HashRange::CopyFixed(reinterpret_cast<const uint8_t*>(message.HashesPtr()), message.Data().HashesCount);
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
			EXPECT_EQ(CurrentTraits::Base_Size, pMessage->Size);

			EXPECT_EQ(0u, pMessage->FinalizationMessage_Reserved1);
			EXPECT_EQ(1u, pMessage->SignatureScheme);
			EXPECT_EQ(FinalizationMessage::Current_Version, pMessage->Data().Version);
			EXPECT_EQ(0u, pMessage->Data().HashesCount);

			EXPECT_EQ(DefaultStepIdentifier(), pMessage->Data().StepIdentifier);
			EXPECT_EQ(Height(987), pMessage->Data().Height);
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
			EXPECT_EQ(CurrentTraits::Base_Size + 3 * Hash256::Size, pMessage->Size);

			EXPECT_EQ(0u, pMessage->FinalizationMessage_Reserved1);
			EXPECT_EQ(FinalizationMessage::Current_Version, pMessage->Data().Version);
			EXPECT_EQ(3u, pMessage->Data().HashesCount);

			EXPECT_EQ(DefaultStepIdentifier(), pMessage->Data().StepIdentifier);
			EXPECT_EQ(Height(987), pMessage->Data().Height);
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
			EXPECT_EQ(CurrentTraits::Base_Size + 3 * Hash256::Size, pMessage->Size);
			EXPECT_EQ(3u, pMessage->Data().HashesCount);

			EXPECT_EQ(DefaultStepIdentifier(), pMessage->Data().StepIdentifier);
			EXPECT_EQ(Height(987), pMessage->Data().Height);
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
		template<typename TTraits, typename TAction>
		void RunProcessMessageTest(
				VoterType voterType,
				uint32_t numHashes,
				const consumer<FinalizationMessage&>& modifyMessage,
				TAction action) {
			// Arrange:
			RunFinalizationContextTest([voterType, numHashes, modifyMessage, action](const auto& context, const auto& keyPairDescriptors) {
				const auto& keyPairDescriptor = keyPairDescriptors[utils::to_underlying_type(voterType)];

				// - create message
				auto pMessage = TTraits::CreateMessage(numHashes);
				pMessage->FinalizationMessage_Reserved1 = 0;
				pMessage->Data().Version = FinalizationMessage::Current_Version;
				pMessage->Data().StepIdentifier = DefaultStepIdentifier();
				pMessage->Data().Height = Height(987);
				modifyMessage(*pMessage);
				test::SignMessage(*pMessage, keyPairDescriptor.VotingKeyPair);

				// Act + Assert:
				action(context, keyPairDescriptor, *pMessage);
			});
		}

		template<typename TTraits, typename TAction>
		void RunProcessMessageTest(VoterType voterType, uint32_t numHashes, TAction action) {
			RunProcessMessageTest<TTraits>(voterType, numHashes, [](const auto&){}, action);
		}
	}

	VERSION_TRAITS(ProcessMessage_FailsWhenSignatureIsInvalid) {
		// Arrange:
		RunProcessMessageTest<TTraits>(VoterType::Large, 3, [](const auto& context, const auto&, auto& message) {
			// - corrupt a hash
			test::FillWithRandomData(message.HashesPtr()[1]);

			// Act:
			auto processResultPair = ProcessMessage(message, context);

			// Assert:
			EXPECT_EQ(ProcessMessageResult::Failure_Signature, processResultPair.first);
			EXPECT_EQ(0u, processResultPair.second);
		});
	}

	VERSION_TRAITS(ProcessMessage_FailsWhenReservedDataIsNotCleared) {
		// Arrange:
		auto modifyMessage = [](auto& message) { message.FinalizationMessage_Reserved1 = 1; };
		RunProcessMessageTest<TTraits>(VoterType::Large, 3, modifyMessage, [](const auto& context, const auto&, const auto& message) {
			// Act:
			auto processResultPair = ProcessMessage(message, context);

			// Assert:
			EXPECT_EQ(ProcessMessageResult::Failure_Padding, processResultPair.first);
			EXPECT_EQ(0u, processResultPair.second);
		});
	}

	VERSION_TRAITS(ProcessMessage_FailsWhenVersionIsIncorrect) {
		// Arrange:
		auto modifyMessage = [](auto& message) { ++message.Data().Version; };
		RunProcessMessageTest<TTraits>(VoterType::Large, 3, modifyMessage, [](const auto& context, const auto&, const auto& message) {
			// Act:
			auto processResultPair = ProcessMessage(message, context);

			// Assert:
			EXPECT_EQ(ProcessMessageResult::Failure_Version, processResultPair.first);
			EXPECT_EQ(0u, processResultPair.second);
		});
	}

	VERSION_TRAITS(ProcessMessage_FailsWhenAccountIsNotVotingEligible) {
		// Arrange:
		RunProcessMessageTest<TTraits>(VoterType::Ineligible, 3, [](const auto& context, const auto&, const auto& message) {
			// Act:
			auto processResultPair = ProcessMessage(message, context);

			// Assert:
			EXPECT_EQ(ProcessMessageResult::Failure_Voter, processResultPair.first);
			EXPECT_EQ(0u, processResultPair.second);
		});
	}

	VERSION_TRAITS(ProcessMessage_CanProcessValidMessageWithoutHashes) {
		// Arrange:
		RunProcessMessageTest<TTraits>(VoterType::Large, 0, [](const auto& context, const auto&, const auto& message) {
			// Act:
			auto processResultPair = ProcessMessage(message, context);

			// Assert:
			EXPECT_EQ(ProcessMessageResult::Success, processResultPair.first);
			EXPECT_EQ(Expected_Large_Weight, processResultPair.second);

			// Sanity:
			EXPECT_EQ(0u, message.Data().HashesCount);
		});
	}

	VERSION_TRAITS(ProcessMessage_CanProcessValidMessageWithHashes) {
		// Arrange:
		RunProcessMessageTest<TTraits>(VoterType::Large, 3, [](const auto& context, const auto&, const auto& message) {
			// Act:
			auto processResultPair = ProcessMessage(message, context);

			// Assert:
			EXPECT_EQ(ProcessMessageResult::Success, processResultPair.first);
			EXPECT_EQ(Expected_Large_Weight, processResultPair.second);

			// Sanity:
			EXPECT_EQ(3u, message.Data().HashesCount);
		});
	}

	// endregion
}}

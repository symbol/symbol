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

#include "finalization/src/chain/FinalizationProofVerifier.h"
#include "finalization/src/model/FinalizationProofUtils.h"
#include "finalization/tests/test/FinalizationMessageTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

#define TEST_CLASS FinalizationProofVerifierTests
#define TEST_CLASS_V1 FinalizationProofVerifierTests_V1

	namespace {
		constexpr auto Finalization_Epoch = FinalizationEpoch(2);
		constexpr auto Finalization_Point = FinalizationPoint(3);
		constexpr auto Last_Finalized_Height = Height(123);

		// region TestContext

		class TestContext {
		public:
			TestContext() {
				auto config = finalization::FinalizationConfiguration::Uninitialized();
				config.Size = 1000;
				config.Threshold = 700;
				config.MaxHashesPerPoint = 100;
				config.VotingSetGrouping = 500;

				// 15/20M voting eligible
				auto finalizationContextPair = test::CreateFinalizationContext(config, Finalization_Epoch, Last_Finalized_Height, {
					Amount(4'000'000), Amount(2'000'000), Amount(1'000'000), Amount(2'000'000), Amount(3'000'000), Amount(4'000'000),
					Amount(1'000'000), Amount(1'000'000), Amount(1'000'000), Amount(1'000'000)
				});

				m_pFinalizationContext = std::make_unique<model::FinalizationContext>(std::move(finalizationContextPair.first));
				m_keyPairDescriptors = std::move(finalizationContextPair.second);
			}

		public:
			auto verify(const model::FinalizationProof& proof) const {
				return VerifyFinalizationProof(proof, *m_pFinalizationContext);
			}

		public:
			void signAllMessages(
					std::vector<std::shared_ptr<model::FinalizationMessage>>& messages,
					const std::vector<size_t>& signerIndexes) const {
				for (auto i = 0u; i < messages.size(); ++i)
					test::SignMessage(*messages[i], m_keyPairDescriptors[signerIndexes[i]].VotingKeyPair);
			}

		private:
			std::unique_ptr<model::FinalizationContext> m_pFinalizationContext;
			std::vector<test::AccountKeyPairDescriptor> m_keyPairDescriptors;
		};

		// endregion

		// region test utils

		using MutableMessages = std::vector<std::shared_ptr<model::FinalizationMessage>>;

		model::FinalizationStatistics CreateFinalizationStatistics(Height heightDelta, const Hash256& hash) {
			return { { Finalization_Epoch, Finalization_Point }, Last_Finalized_Height + heightDelta, hash };
		}

		auto MergeMessages(const MutableMessages& lhs, const MutableMessages& rhs) {
			std::vector<std::shared_ptr<const model::FinalizationMessage>> messages;
			messages.insert(messages.end(), lhs.cbegin(), lhs.cend());
			messages.insert(messages.end(), rhs.cbegin(), rhs.cend());
			return messages;
		}

		// endregion

		// region RunTest

		template<typename TTraits, typename TAction>
		void RunTest(TAction action) {
			// Arrange: only setup a prevote on the first 6/7 hashes
			auto prevoteHashes = test::GenerateRandomDataVector<Hash256>(7);
			auto prevoteMessages = TTraits::CreatePrevoteMessages(4, prevoteHashes.data(), prevoteHashes.size());
			prevoteMessages.back()->Size -= static_cast<uint32_t>(Hash256::Size);
			--prevoteMessages.back()->Data().HashesCount;

			// - only setup a precommit on the first 3/7 hashes
			auto precommitMessages = TTraits::CreatePrecommitMessages(4, prevoteHashes.data(), 3);
			precommitMessages.back()->Data().Height = precommitMessages.back()->Data().Height - Height(1);
			*precommitMessages.back()->HashesPtr() = prevoteHashes[2];

			// - sign prevotes with weights { 4M, 2M, 3M, 4M } (13M) > 15M * 0.7 (10.5M)
			// - sign precommits with weights { 2M, 2M, 4M, 3M } (11M) > 15M * 0.7 (10.5M)
			TestContext context;
			context.signAllMessages(prevoteMessages, { 5, 1, 4, 0 });
			context.signAllMessages(precommitMessages, { 3, 1, 0, 4 });

			// Act + Assert: run test
			action(context, prevoteHashes, prevoteMessages, precommitMessages);
		}

		template<typename TTraits, typename TModifier>
		void RunModifiedStatisticsTest(VerifyFinalizationProofResult expectedResult, TModifier modifyStatistics) {
			// Arrange:
			RunTest<TTraits>([expectedResult, modifyStatistics](
					const auto& context,
					const auto& prevoteHashes,
					const auto& prevoteMessages,
					const auto& precommitMessages) {
				auto statistics = CreateFinalizationStatistics(Height(3), prevoteHashes[2]);
				modifyStatistics(statistics);
				auto pProof = model::CreateFinalizationProof(statistics, MergeMessages(prevoteMessages, precommitMessages));

				// Act: verify it
				auto result = context.verify(*pProof);

				// Assert:
				EXPECT_EQ(expectedResult, result);
			});
		}

		// endregion
	}

	// region traits

	namespace {
		struct CurrentTraits {
			static auto CreatePrevoteMessages(size_t numMessages, const Hash256* pHashes, size_t numHashes) {
				auto epoch = Finalization_Epoch;
				auto point = Finalization_Point;
				auto height = Last_Finalized_Height + Height(1);
				return test::CreatePrevoteMessages(epoch, point, height, numMessages, pHashes, numHashes);
			}

			static auto CreatePrecommitMessages(size_t numMessages, const Hash256* pHashes, size_t index) {
				auto epoch = Finalization_Epoch;
				auto point = Finalization_Point;
				auto height = Last_Finalized_Height + Height(1);
				return test::CreatePrecommitMessages(epoch, point, height, numMessages, pHashes, index);
			}
		};

		struct V1Traits {
			static auto CreatePrevoteMessages(size_t numMessages, const Hash256* pHashes, size_t numHashes) {
				auto epoch = Finalization_Epoch;
				auto point = Finalization_Point;
				auto height = Last_Finalized_Height + Height(1);

				std::vector<std::shared_ptr<model::FinalizationMessage>> messages;
				for (auto i = 0u; i < numMessages; ++i) {
					auto pMessage = test::CreateMessageV1(height, static_cast<uint32_t>(numHashes));
					pMessage->Data().StepIdentifier = { epoch, point, model::FinalizationStage::Prevote };
					std::copy(pHashes, pHashes + numHashes, pMessage->HashesPtr());
					messages.push_back(std::move(pMessage));
				}

				return messages;
			}

			static auto CreatePrecommitMessages(size_t numMessages, const Hash256* pHashes, size_t index) {
				auto epoch = Finalization_Epoch;
				auto point = Finalization_Point;
				auto height = Last_Finalized_Height + Height(1);

				std::vector<std::shared_ptr<model::FinalizationMessage>> messages;
				for (auto i = 0u; i < numMessages; ++i) {
					auto pMessage = test::CreateMessage(height + Height(index), 1);
					pMessage->Data().StepIdentifier = { epoch, point, model::FinalizationStage::Precommit };
					*pMessage->HashesPtr() = pHashes[index];
					messages.push_back(std::move(pMessage));
				}

				return messages;
			}
		};
	}

#define VERSION_TRAITS(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CurrentTraits>(); } \
	TEST(TEST_CLASS_V1, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<V1Traits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region failure

	VERSION_TRAITS(VerifyFailsWhenProofHasUnsupportedVersion) {
		// Arrange:
		RunTest<TTraits>([](const auto& context, const auto& prevoteHashes, const auto& prevoteMessages, const auto& precommitMessages) {
			auto statistics = CreateFinalizationStatistics(Height(3), prevoteHashes[2]);
			auto pProof = model::CreateFinalizationProof(statistics, MergeMessages(prevoteMessages, precommitMessages));

			// - change version
			++pProof->Version;

			// Act: verify it
			auto result = context.verify(*pProof);

			// Assert:
			EXPECT_EQ(VerifyFinalizationProofResult::Failure_Invalid_Version, result);
		});
	}

	VERSION_TRAITS(VerifyFailsWhenProofEpochDoesNotMatchContextEpoch) {
		RunModifiedStatisticsTest<TTraits>(VerifyFinalizationProofResult::Failure_Invalid_Epoch, [](auto& statistics) {
			statistics.Round.Epoch = statistics.Round.Epoch + FinalizationEpoch(1);
		});
	}

	VERSION_TRAITS(VerifyFailsWhenProofHeightDoesNotMatchCalculatedHeight) {
		RunModifiedStatisticsTest<TTraits>(VerifyFinalizationProofResult::Failure_Invalid_Height, [](auto& statistics) {
			statistics.Height = statistics.Height + Height(1);
		});
	}

	VERSION_TRAITS(VerifyFailsWhenProofHashDoesNotMatchCalculatedHash) {
		RunModifiedStatisticsTest<TTraits>(VerifyFinalizationProofResult::Failure_Invalid_Hash, [](auto& statistics) {
			test::FillWithRandomData(statistics.Hash);
		});
	}

	VERSION_TRAITS(VerifyFailsWhenProofDoesNotProveAnything) {
		// Arrange:
		RunTest<TTraits>([](const auto& context, const auto& prevoteHashes, const auto& prevoteMessages, auto& precommitMessages) {
			// - drop precommit message
			precommitMessages.pop_back();

			auto statistics = CreateFinalizationStatistics(Height(3), prevoteHashes[2]);
			auto pProof = model::CreateFinalizationProof(statistics, MergeMessages(prevoteMessages, precommitMessages));

			// Act: verify it
			auto result = context.verify(*pProof);

			// Assert:
			EXPECT_EQ(VerifyFinalizationProofResult::Failure_No_Precommit, result);
		});
	}

	namespace {
		template<typename TTraits, typename TModifier>
		void RunModifiedPrevoteMessagesTest(TModifier modifyPrevoteMessages) {
			// Arrange:
			RunTest<TTraits>([modifyPrevoteMessages](
					const auto& context,
					const auto& prevoteHashes,
					auto& prevoteMessages,
					const auto& precommitMessages) {
				modifyPrevoteMessages(prevoteMessages);

				auto statistics = CreateFinalizationStatistics(Height(3), prevoteHashes[2]);
				auto pProof = model::CreateFinalizationProof(statistics, MergeMessages(prevoteMessages, precommitMessages));

				// Act: verify it
				auto result = context.verify(*pProof);

				// Assert:
				EXPECT_EQ(VerifyFinalizationProofResult::Failure_Invalid_Messsage, result);
			});
		}
	}

	VERSION_TRAITS(VerifyFailsWhenProofContainsRedundantMessage) {
		// Arrange:
		RunModifiedPrevoteMessagesTest<TTraits>([](auto& prevoteMessages) {
			// - duplicate prevote message
			prevoteMessages.push_back(prevoteMessages.front());
		});
	}

	VERSION_TRAITS(VerifyFailsWhenProofContainsMessageThatFailsProcessing) {
		// Arrange:
		RunModifiedPrevoteMessagesTest<TTraits>([](auto& prevoteMessages) {
			// - corrupt signature
			prevoteMessages[prevoteMessages.size() / 2]->Signature().Root.ParentPublicKey[0] ^= 0xFF;
		});
	}

	// endregion

	// region success

	VERSION_TRAITS(CanVerifyProofWithMultipleMessageGroupsWithMultipleMessages) {
		RunModifiedStatisticsTest<TTraits>(VerifyFinalizationProofResult::Success, [](const auto&) {});
	}

	// endregion
}}

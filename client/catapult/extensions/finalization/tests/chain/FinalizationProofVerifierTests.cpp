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

#include "finalization/src/chain/FinalizationProofVerifier.h"
#include "finalization/src/model/FinalizationProofUtils.h"
#include "finalization/tests/test/FinalizationMessageTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

#define TEST_CLASS FinalizationProofVerifierTests

	namespace {
		constexpr auto Finalization_Point = FinalizationPoint(3);
		constexpr auto Last_Finalized_Height = Height(123);

		// region TestContext

		class TestContext {
		private:
			static constexpr auto Ots_Key_Dilution = 7u;

		public:
			TestContext() {
				auto config = finalization::FinalizationConfiguration::Uninitialized();
				config.Size = 1000;
				config.Threshold = 700;
				config.MaxHashesPerPoint = 100;
				config.OtsKeyDilution = Ots_Key_Dilution;
				config.VotingSetGrouping = 500;

				// 15/20M voting eligible
				auto finalizationContextPair = test::CreateFinalizationContext(config, Finalization_Point, Last_Finalized_Height, {
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
					test::SignMessage(*messages[i], m_keyPairDescriptors[signerIndexes[i]].VotingKeyPair, Ots_Key_Dilution);
			}

		private:
			std::unique_ptr<model::FinalizationContext> m_pFinalizationContext;
			std::vector<test::AccountKeyPairDescriptor> m_keyPairDescriptors;
		};

		// endregion

		// region test utils

		using MutableMessages = std::vector<std::shared_ptr<model::FinalizationMessage>>;

		auto CreatePrevoteMessages(size_t numMessages, const Hash256* pHashes, size_t numHashes) {
			return test::CreatePrevoteMessages(Finalization_Point, Last_Finalized_Height + Height(1), numMessages, pHashes, numHashes);
		}

		auto CreatePrecommitMessages(size_t numMessages, const Hash256* pHashes, size_t index) {
			return test::CreatePrecommitMessages(Finalization_Point, Last_Finalized_Height + Height(1), numMessages, pHashes, index);
		}

		auto MergeMessages(const MutableMessages& lhs, const MutableMessages& rhs) {
			std::vector<std::shared_ptr<const model::FinalizationMessage>> messages;
			messages.insert(messages.end(), lhs.cbegin(), lhs.cend());
			messages.insert(messages.end(), rhs.cbegin(), rhs.cend());
			return messages;
		}

		// endregion

		// region RunTest

		template<typename TAction>
		void RunTest(TAction action) {
			// Arrange: only setup a prevote on the first 6/7 hashes
			auto prevoteHashes = test::GenerateRandomDataVector<Hash256>(7);
			auto prevoteMessages = CreatePrevoteMessages(4, prevoteHashes.data(), prevoteHashes.size());
			prevoteMessages.back()->Size -= static_cast<uint32_t>(Hash256::Size);
			--prevoteMessages.back()->HashesCount;

			// - only setup a precommit on the first 3/7 hashes
			auto precommitMessages = CreatePrecommitMessages(4, prevoteHashes.data(), 3);
			precommitMessages.back()->Height = precommitMessages.back()->Height - Height(1);
			*precommitMessages.back()->HashesPtr() = prevoteHashes[2];

			// - sign prevotes with weights { 4M, 2M, 3M, 4M } (13M) > 15M * 0.7 (10.5M)
			// - sign precommits with weights { 2M, 2M, 4M, 3M } (11M) > 15M * 0.7 (10.5M)
			TestContext context;
			context.signAllMessages(prevoteMessages, { 5, 1, 4, 0 });
			context.signAllMessages(precommitMessages, { 3, 1, 0, 4 });

			// Act + Assert: run test
			action(context, prevoteHashes, prevoteMessages, precommitMessages);
		}

		template<typename TModifier>
		void RunModifiedStatisticsTest(VerifyFinalizationProofResult expectedResult, TModifier modifyStatistics) {
			// Arrange:
			RunTest([expectedResult, modifyStatistics](
					const auto& context,
					const auto& prevoteHashes,
					const auto& prevoteMessages,
					const auto& precommitMessages) {
				auto statistics = model::FinalizationStatistics{ Finalization_Point, Last_Finalized_Height + Height(3), prevoteHashes[2] };
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

	// region failure

	TEST(TEST_CLASS, VerifyFailsWhenProofHasUnsupportedVersion) {
		// Arrange:
		RunTest([](const auto& context, const auto& prevoteHashes, const auto& prevoteMessages, const auto& precommitMessages) {
			auto statistics = model::FinalizationStatistics{ Finalization_Point, Last_Finalized_Height + Height(3), prevoteHashes[2] };
			auto pProof = model::CreateFinalizationProof(statistics, MergeMessages(prevoteMessages, precommitMessages));

			// - change version
			++pProof->Version;

			// Act: verify it
			auto result = context.verify(*pProof);

			// Assert:
			EXPECT_EQ(VerifyFinalizationProofResult::Failure_Invalid_Version, result);
		});
	}

	TEST(TEST_CLASS, VerifyFailsWhenProofPointDoesNotMatchContextPoint) {
		RunModifiedStatisticsTest(VerifyFinalizationProofResult::Failure_Invalid_Point, [](auto& statistics) {
			statistics.Point = statistics.Point + FinalizationPoint(1);
		});
	}

	TEST(TEST_CLASS, VerifyFailsWhenProofHeightDoesNotMatchCalculatedHeight) {
		RunModifiedStatisticsTest(VerifyFinalizationProofResult::Failure_Invalid_Height, [](auto& statistics) {
			statistics.Height = statistics.Height + Height(1);
		});
	}

	TEST(TEST_CLASS, VerifyFailsWhenProofHashDoesNotMatchCalculatedHash) {
		RunModifiedStatisticsTest(VerifyFinalizationProofResult::Failure_Invalid_Hash, [](auto& statistics) {
			test::FillWithRandomData(statistics.Hash);
		});
	}

	TEST(TEST_CLASS, VerifyFailsWhenProofDoesNotProveAnything) {
		// Arrange:
		RunTest([](const auto& context, const auto& prevoteHashes, const auto& prevoteMessages, auto& precommitMessages) {
			// - drop precommit message
			precommitMessages.pop_back();

			auto statistics = model::FinalizationStatistics{ Finalization_Point, Last_Finalized_Height + Height(3), prevoteHashes[2] };
			auto pProof = model::CreateFinalizationProof(statistics, MergeMessages(prevoteMessages, precommitMessages));

			// Act: verify it
			auto result = context.verify(*pProof);

			// Assert:
			EXPECT_EQ(VerifyFinalizationProofResult::Failure_No_Precommit, result);
		});
	}

	namespace {
		template<typename TModifier>
		void RunModifiedPrevoteMessagesTest(TModifier modifyPrevoteMessages) {
			// Arrange:
			RunTest([modifyPrevoteMessages](
					const auto& context,
					const auto& prevoteHashes,
					auto& prevoteMessages,
					const auto& precommitMessages) {
				modifyPrevoteMessages(prevoteMessages);

				auto statistics = model::FinalizationStatistics{ Finalization_Point, Last_Finalized_Height + Height(3), prevoteHashes[2] };
				auto pProof = model::CreateFinalizationProof(statistics, MergeMessages(prevoteMessages, precommitMessages));

				// Act: verify it
				auto result = context.verify(*pProof);

				// Assert:
				EXPECT_EQ(VerifyFinalizationProofResult::Failure_Invalid_Messsage, result);
			});
		}
	}

	TEST(TEST_CLASS, VerifyFailsWhenProofContainsRedundantMessage) {
		// Arrange:
		RunModifiedPrevoteMessagesTest([](auto& prevoteMessages) {
			// - duplicate prevote message
			prevoteMessages.push_back(prevoteMessages.front());
		});
	}

	TEST(TEST_CLASS, VerifyFailsWhenProofContainsMessageThatFailsProcessing) {
		// Arrange:
		RunModifiedPrevoteMessagesTest([](auto& prevoteMessages) {
			// - corrupt signature
			prevoteMessages[prevoteMessages.size() / 2]->Signature.Root.ParentPublicKey[0] ^= 0xFF;
		});
	}

	// endregion

	// region success

	TEST(TEST_CLASS, CanVerifyProofWithMultipleMessageGroupsWithMultipleMessages) {
		RunModifiedStatisticsTest(VerifyFinalizationProofResult::Success, [](const auto&) {});
	}

	// endregion
}}

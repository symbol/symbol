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

#include "finalization/src/chain/FinalizationProofSynchronizer.h"
#include "finalization/src/io/ProofStorageCache.h"
#include "catapult/io/BlockStorageCache.h"
#include "finalization/tests/test/FinalizationMessageTestUtils.h"
#include "finalization/tests/test/mocks/MockProofApi.h"
#include "finalization/tests/test/mocks/MockProofStorage.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

#define TEST_CLASS FinalizationProofSynchronizerTests

	namespace {
		// region TestContext

		class TestContext {
		public:
			TestContext(
					uint64_t votingSetGrouping,
					Height localChainHeight,
					Height localFinalizedHeight,
					BlockDuration unfinalizedBlocksDuration = BlockDuration())
					: m_pBlockStorageCache(mocks::CreateMemoryBlockStorageCache(static_cast<uint32_t>(localChainHeight.unwrap())))
					, m_pProofStorage(std::make_unique<mocks::MockProofStorage>(FinalizationPoint(11), localFinalizedHeight))
					, m_pProofStorageRaw(m_pProofStorage.get())
					, m_proofStorageCache(std::move(m_pProofStorage))
					, m_synchronizer(CreateFinalizationProofSynchronizer(
							votingSetGrouping,
							unfinalizedBlocksDuration,
							*m_pBlockStorageCache,
							m_proofStorageCache,
							[this](const auto&) {
								++m_numValidationCalls;
								return m_validationResult;
							}))
					, m_numValidationCalls(0)
					, m_validationResult(true)
			{}

		public:
			auto& api() {
				return m_proofApi;
			}

			auto& storage() {
				return *m_pProofStorageRaw;
			}

			auto numValidationCalls() {
				return m_numValidationCalls;
			}

		public:
			void setValidationFailure() {
				m_validationResult = false;
			}

		public:
			auto synchronize() {
				return m_synchronizer(m_proofApi).get();
			}

		private:
			std::unique_ptr<io::BlockStorageCache> m_pBlockStorageCache;

			std::unique_ptr<mocks::MockProofStorage> m_pProofStorage; // moved into m_proofStorageCache
			mocks::MockProofStorage* m_pProofStorageRaw;
			io::ProofStorageCache m_proofStorageCache;

			RemoteNodeSynchronizer<api::RemoteProofApi> m_synchronizer;

			mocks::MockProofApi m_proofApi;
			size_t m_numValidationCalls;
			bool m_validationResult;
		};

		// endregion
	}

	// region bypass - proof already pulled

	namespace {
		void AssertNeutral(TestContext& context) {
			// Act:
			auto result = context.synchronize();

			// Assert:
			EXPECT_EQ(ionet::NodeInteractionResultCode::Neutral, result);

			EXPECT_TRUE(context.api().proofEpochs().empty());
			EXPECT_EQ(0u, context.numValidationCalls());
			EXPECT_TRUE(context.storage().savedProofDescriptors().empty());
		}

		void AssertApiIsBypassed(Height localChainHeight) {
			// Arrange:
			TestContext context(20, localChainHeight, Height(81));

			// Act + Assert:
			AssertNeutral(context);
		}
	}

	TEST(TEST_CLASS, NeutralWhenNextProofHeightIsGreaterThanLocalChainHeight) {
		AssertApiIsBypassed(Height(35));
		AssertApiIsBypassed(Height(70));
		AssertApiIsBypassed(Height(99));
	}

	TEST(TEST_CLASS, NeutralWhenNextProofHeightIsEqualToLocalChainHeight) {
		AssertApiIsBypassed(Height(100));
	}

	// endregion

	// region api errors + short circuit

	namespace {
		model::FinalizationStatistics CreateFinalizationStatistics(FinalizationEpoch epoch) {
			return { { epoch, FinalizationPoint(12) }, Height(100), Hash256() };
		}
	}

	TEST(TEST_CLASS, FailureWhenFinalizationStatisticsFails) {
		// Arrange:
		TestContext context(20, Height(101), Height(81));
		context.api().setError(mocks::MockProofApi::EntryPoint::Finalization_Statistics);

		// Act:
		auto result = context.synchronize();

		// Assert:
		EXPECT_EQ(ionet::NodeInteractionResultCode::Failure, result);

		EXPECT_TRUE(context.api().proofEpochs().empty());
		EXPECT_EQ(0u, context.numValidationCalls());
		EXPECT_TRUE(context.storage().savedProofDescriptors().empty());
	}

	TEST(TEST_CLASS, NeutralWhenFinalizationStatisticsReturnsEpochLessThanRequestEpoch) {
		// Arrange:
		TestContext context(20, Height(101), Height(81));
		context.api().setFinalizationStatistics(CreateFinalizationStatistics(FinalizationEpoch(4)));

		// Act + Assert:
		AssertNeutral(context);
	}

	TEST(TEST_CLASS, FailureWhenProofAtFails) {
		// Arrange:
		TestContext context(20, Height(101), Height(81));
		context.api().setFinalizationStatistics(CreateFinalizationStatistics(FinalizationEpoch(6)));
		context.api().setError(mocks::MockProofApi::EntryPoint::Proof_At_Epoch);

		// Act:
		auto result = context.synchronize();

		// Assert:
		EXPECT_EQ(ionet::NodeInteractionResultCode::Failure, result);

		EXPECT_EQ(std::vector<FinalizationEpoch>({ FinalizationEpoch(6) }), context.api().proofEpochs());
		EXPECT_EQ(0u, context.numValidationCalls());
		EXPECT_TRUE(context.storage().savedProofDescriptors().empty());
	}

	// endregion

	// region proof validation failures

	namespace {
		void SetProofHeader(model::FinalizationProof& proof, const model::FinalizationStatistics& finalizationStatistics) {
			proof.Round = finalizationStatistics.Round;
			proof.Height = finalizationStatistics.Height;
			proof.Hash = finalizationStatistics.Hash;
		}
	}

	TEST(TEST_CLASS, FailureWhenRemoteProofEpochDoesNotMatchRequestedProofEpoch) {
		// Arrange:
		TestContext context(20, Height(101), Height(81));
		context.api().setFinalizationStatistics(CreateFinalizationStatistics(FinalizationEpoch(6)));

		// - epoch is off by one
		auto pProof = std::make_shared<model::FinalizationProof>();
		SetProofHeader(*pProof, CreateFinalizationStatistics(FinalizationEpoch(7)));
		context.api().setProof(pProof);

		// Act:
		auto result = context.synchronize();

		// Assert:
		EXPECT_EQ(ionet::NodeInteractionResultCode::Failure, result);

		EXPECT_EQ(std::vector<FinalizationEpoch>({ FinalizationEpoch(6) }), context.api().proofEpochs());
		EXPECT_EQ(0u, context.numValidationCalls());
		EXPECT_TRUE(context.storage().savedProofDescriptors().empty());
	}

	TEST(TEST_CLASS, FailureWhenRemoteProofFailsValidation) {
		// Arrange:
		TestContext context(20, Height(101), Height(81));
		context.api().setFinalizationStatistics(CreateFinalizationStatistics(FinalizationEpoch(6)));
		context.setValidationFailure();

		auto pProof = std::make_shared<model::FinalizationProof>();
		SetProofHeader(*pProof, CreateFinalizationStatistics(FinalizationEpoch(6)));
		context.api().setProof(pProof);

		// Act:
		auto result = context.synchronize();

		// Assert:
		EXPECT_EQ(ionet::NodeInteractionResultCode::Failure, result);

		EXPECT_EQ(std::vector<FinalizationEpoch>({ FinalizationEpoch(6) }), context.api().proofEpochs());
		EXPECT_EQ(1u, context.numValidationCalls());
		EXPECT_TRUE(context.storage().savedProofDescriptors().empty());
	}

	// endregion

	// region success

	namespace {
		void AssertSuccess(TestContext& context, FinalizationEpoch proofEpoch) {
			// Arrange:
			auto pProof = std::make_shared<model::FinalizationProof>();
			pProof->Round = { proofEpoch, FinalizationPoint(111) };
			pProof->Height = Height(234);
			pProof->Hash = Hash256{ { 33 } };
			context.api().setProof(pProof);

			// Act:
			auto result = context.synchronize();

			// Assert:
			EXPECT_EQ(ionet::NodeInteractionResultCode::Success, result);

			EXPECT_EQ(std::vector<FinalizationEpoch>({ proofEpoch }), context.api().proofEpochs());
			EXPECT_EQ(1u, context.numValidationCalls());

			ASSERT_EQ(1u, context.storage().savedProofDescriptors().size());
			const auto& savedProofDescriptor = context.storage().savedProofDescriptors()[0];
			EXPECT_EQ(test::CreateFinalizationRound(proofEpoch.unwrap(), 111), savedProofDescriptor.Round);
			EXPECT_EQ(Height(234), savedProofDescriptor.Height);
			EXPECT_EQ(Hash256{ { 33 } }, savedProofDescriptor.Hash);
		}

		void AssertSuccess(Height localChainHeight, FinalizationEpoch remoteEpoch) {
			// Arrange:
			TestContext context(20, localChainHeight, Height(81));
			context.api().setFinalizationStatistics(CreateFinalizationStatistics(remoteEpoch));

			// Act + Assert:
			AssertSuccess(context, FinalizationEpoch(6));
		}
	}

	TEST(TEST_CLASS, SuccessWhenRemoteFinalizationStatisticsReturnsEpochEqualToExpectedProofEpoch) {
		// Assert: returned proof is for epoch 6
		AssertSuccess(Height(101), FinalizationEpoch(6));
	}

	TEST(TEST_CLASS, SuccessWhenRemoteFinalizationStatisticsReturnsEpochGreaterThanExpectedProofEpoch) {
		// Assert: returned proof is for epoch 6 even though remote has up to epoch 11
		AssertSuccess(Height(101), FinalizationEpoch(11));
	}

	TEST(TEST_CLASS, SuccessWhenLocalChainHeightIsMultipleVotingSetsAheadOfLocalFinalizedHeight) {
		// Assert: returned proof is for epoch 6 even though remote has up to epoch 11 and local has up to epoch 11 unfinalized blocks
		AssertSuccess(Height(201), FinalizationEpoch(11));
	}

	namespace {
		void AssertSuccessNext(Height localChainHeight, Height localFinalizedHeight, FinalizationEpoch expectedProofEpoch) {
			// Arrange:
			TestContext context(20, localChainHeight, localFinalizedHeight);
			context.api().setFinalizationStatistics(CreateFinalizationStatistics(expectedProofEpoch));

			// Act + Assert:
			AssertSuccess(context, expectedProofEpoch);
		}
	}

	TEST(TEST_CLASS, SuccessWhenRequestingNextProof_FromNemesisEpoch) {
		AssertSuccessNext(Height(21), Height(1), FinalizationEpoch(2));
	}

	TEST(TEST_CLASS, SuccessWhenRequestingNextProof_FromOtherEpoch) {
		AssertSuccessNext(Height(41), Height(20), FinalizationEpoch(3));
	}

	// endregion

	// region unfinalizedBlocksDuration

	TEST(TEST_CLASS, NeutralWhenLocalUnfinalizedBlocksDurationIsLessThanThreshold_NonzeroUnfinalizedBlocksDuration) {
		// Arrange:
		TestContext context(20, Height(85), Height(81), BlockDuration(5));
		context.api().setFinalizationStatistics(CreateFinalizationStatistics(FinalizationEpoch(6)));

		// Act + Assert:
		AssertNeutral(context);
	}

	TEST(TEST_CLASS, SuccessWhenLocalUnfinalizedBlocksDurationIsEqualToThreshold_NonzeroUnfinalizedBlocksDuration) {
		// Arrange:
		TestContext context(20, Height(86), Height(81), BlockDuration(5));
		context.api().setFinalizationStatistics(CreateFinalizationStatistics(FinalizationEpoch(6)));

		// Act + Assert:
		AssertSuccess(context, FinalizationEpoch(6));
	}

	TEST(TEST_CLASS, SuccessWhenLocalUnfinalizedBlocksDurationIsGreaterThanThreshold_NonzeroUnfinalizedBlocksDuration) {
		// Arrange:
		TestContext context(20, Height(99), Height(81), BlockDuration(5));
		context.api().setFinalizationStatistics(CreateFinalizationStatistics(FinalizationEpoch(6)));

		// Act + Assert:
		AssertSuccess(context, FinalizationEpoch(6));
	}

	TEST(TEST_CLASS, SuccessWhenRequestingNextProof_NonzeroUnfinalizedBlocksDuration) {
		// Arrange:
		TestContext context(20, Height(301), Height(81), BlockDuration(100));
		context.api().setFinalizationStatistics(CreateFinalizationStatistics(FinalizationEpoch(6)));

		// Act + Assert: next end of epoch is pulled
		AssertSuccess(context, FinalizationEpoch(6));
	}

	// endregion
}}

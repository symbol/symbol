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

#include "finalization/src/io/AggregateProofStorage.h"
#include "tests/test/other/mocks/MockFinalizationSubscriber.h"
#include "tests/TestHarness.h"

namespace catapult { namespace io {

#define TEST_CLASS AggregateProofStorageTests

	namespace {
		// region basic mocks

		class UnsupportedProofStorage : public ProofStorage {
		public:
			model::FinalizationStatistics statistics() const override {
				CATAPULT_THROW_RUNTIME_ERROR("statistics - not supported in mock");
			}

			std::shared_ptr<const model::FinalizationProof> loadProof(FinalizationEpoch) const override {
				CATAPULT_THROW_RUNTIME_ERROR("loadProof(FinalizationEpoch) - not supported in mock");
			}

			std::shared_ptr<const model::FinalizationProof> loadProof(Height) const override {
				CATAPULT_THROW_RUNTIME_ERROR("loadProof(Height) - not supported in mock");
			}

			void saveProof(const model::FinalizationProof&) override {
				CATAPULT_THROW_RUNTIME_ERROR("saveProof - not supported in mock");
			}
		};

		class UnsupportedFinalizationSubscriber : public subscribers::FinalizationSubscriber {
		public:
			void notifyFinalizedBlock(const model::FinalizationRound&, Height, const Hash256&) override {
				CATAPULT_THROW_RUNTIME_ERROR("notifyFinalizedBlock - not supported in mock");
			}
		};

		// endregion

		// region test context

		template<typename TProofStorage, typename TFinalizationSubscriber = UnsupportedFinalizationSubscriber>
		class TestContext {
		public:
			TestContext()
					: m_pStorage(std::make_unique<TProofStorage>())
					, m_pStorageRaw(m_pStorage.get())
					, m_pSubscriber(std::make_unique<TFinalizationSubscriber>())
					, m_pSubscriberRaw(m_pSubscriber.get())
					, m_pAggregate(CreateAggregateProofStorage(std::move(m_pStorage), std::move(m_pSubscriber)))
			{}

		public:
			auto& storage() {
				return *m_pStorageRaw;
			}

			auto& subscriber() {
				return *m_pSubscriberRaw;
			}

			auto& aggregate() {
				return *m_pAggregate;
			}

		private:
			std::unique_ptr<TProofStorage> m_pStorage; // notice that this is moved into m_pAggregate
			TProofStorage* m_pStorageRaw;
			std::unique_ptr<TFinalizationSubscriber> m_pSubscriber; // notice that this is moved into m_pAggregate
			TFinalizationSubscriber* m_pSubscriberRaw;
			std::unique_ptr<ProofStorage> m_pAggregate;
		};

		// endregion
	}

	// region statistics

	TEST(TEST_CLASS, StatisticsDelegatesToStorage) {
		// Arrange:
		class MockProofStorage : public UnsupportedProofStorage {
		public:
			mutable size_t NumCalls = 0;

		public:
			model::FinalizationStatistics statistics() const override {
				++NumCalls;
				return { { FinalizationEpoch(7), FinalizationPoint(4) }, Height(123), Hash256() };
			}
		};

		TestContext<MockProofStorage> context;

		// Act:
		auto statistics = context.aggregate().statistics();

		// Assert:
		EXPECT_EQ(1u, context.storage().NumCalls);
		EXPECT_EQ(Height(123), statistics.Height);
	}

	// endregion

	// region loadProof

	namespace {
		class MockProofStorageProofLoader : public UnsupportedProofStorage {
		public:
			mutable std::vector<FinalizationEpoch> Epochs;
			mutable std::vector<Height> Heights;
			std::shared_ptr<const model::FinalizationProof> pProof;

		public:
			MockProofStorageProofLoader() : pProof(std::make_unique<model::FinalizationProof>())
			{}

		public:
			std::shared_ptr<const model::FinalizationProof> loadProof(FinalizationEpoch epoch) const override {
				Epochs.push_back(epoch);
				return pProof;
			}

			std::shared_ptr<const model::FinalizationProof> loadProof(Height height) const override {
				Heights.push_back(height);
				return pProof;
			}
		};
	}

	TEST(TEST_CLASS, LoadProofByEpochDelegatesToStorage) {
		// Arrange:
		TestContext<MockProofStorageProofLoader> context;

		// Act:
		auto pProof = context.aggregate().loadProof(FinalizationEpoch(321));

		// Assert:
		EXPECT_EQ(std::vector<FinalizationEpoch>({ FinalizationEpoch(321) }), context.storage().Epochs);
		EXPECT_EQ(std::vector<Height>(), context.storage().Heights);
		EXPECT_EQ(context.storage().pProof, pProof);
	}

	TEST(TEST_CLASS, LoadProofByHeightDelegatesToStorage) {
		// Arrange:
		TestContext<MockProofStorageProofLoader> context;

		// Act:
		auto pProof = context.aggregate().loadProof(Height(123));

		// Assert:
		EXPECT_EQ(std::vector<FinalizationEpoch>(), context.storage().Epochs);
		EXPECT_EQ(std::vector<Height>({ Height(123) }), context.storage().Heights);
		EXPECT_EQ(context.storage().pProof, pProof);
	}

	// endregion

	// region saveProof

	namespace {
		class MockProofStorage : public UnsupportedProofStorage {
		public:
			std::vector<const model::FinalizationProof*> Proofs;

		public:
			model::FinalizationStatistics statistics() const override {
				model::FinalizationStatistics statistics;
				statistics.Round = { FinalizationEpoch(7), FinalizationPoint(5) };
				return statistics;
			}

			void saveProof(const model::FinalizationProof& proof) override {
				Proofs.push_back(&proof);
			}
		};

		std::unique_ptr<model::FinalizationProof> CreateProofWithRound(const model::FinalizationRound& proofRound) {
			auto pProof = std::make_unique<model::FinalizationProof>();
			pProof->Size = sizeof(model::FinalizationProof);
			pProof->Round = proofRound;
			pProof->Height = Height(246);
			pProof->Hash = test::GenerateRandomByteArray<Hash256>();
			return pProof;
		}

		void AssertSaveProofDelegation(const model::FinalizationRound& proofRound) {
			// Arrange:
			TestContext<MockProofStorage, mocks::MockFinalizationSubscriber> context;

			auto pProof = CreateProofWithRound(proofRound);

			// Act:
			context.aggregate().saveProof(*pProof);

			// Assert:
			ASSERT_EQ(1u, context.storage().Proofs.size());
			EXPECT_EQ(pProof.get(), context.storage().Proofs[0]);

			const auto& subscriberParams = context.subscriber().finalizedBlockParams().params();
			ASSERT_EQ(1u, subscriberParams.size());
			EXPECT_EQ(pProof->Round, subscriberParams[0].Round);
			EXPECT_EQ(pProof->Height, subscriberParams[0].Height);
			EXPECT_EQ(pProof->Hash, subscriberParams[0].Hash);
		}

		void AssertNoSaveProofDelegation(const model::FinalizationRound& proofRound) {
			// Arrange:
			TestContext<MockProofStorage, mocks::MockFinalizationSubscriber> context;

			auto pProof = CreateProofWithRound(proofRound);

			// Act:
			context.aggregate().saveProof(*pProof);

			// Assert:
			EXPECT_EQ(0u, context.storage().Proofs.size());

			EXPECT_EQ(0u, context.subscriber().finalizedBlockParams().params().size());
		}
	}

	TEST(TEST_CLASS, SaveProofDelegatesToStorageAndPublisherWhenLastSavedRoundIsLessThanProofRound) {
		AssertSaveProofDelegation({ FinalizationEpoch(9), FinalizationPoint(0) });
		AssertSaveProofDelegation({ FinalizationEpoch(8), FinalizationPoint(5) });
		AssertSaveProofDelegation({ FinalizationEpoch(7), FinalizationPoint(6) });
	}

	TEST(TEST_CLASS, SaveProofDelegatesToStorageAndPublisherWhenLastSavedRoundIsEqualToProofRound) {
		AssertSaveProofDelegation({ FinalizationEpoch(7), FinalizationPoint(5) });
	}

	TEST(TEST_CLASS, SaveProofDoesNotDelegateToStorageAndPublisherWhenLastSavedRoundIsGreaterThanProofRound) {
		AssertNoSaveProofDelegation({ FinalizationEpoch(7), FinalizationPoint(4) });
		AssertNoSaveProofDelegation({ FinalizationEpoch(6), FinalizationPoint(5) });
		AssertNoSaveProofDelegation({ FinalizationEpoch(5), FinalizationPoint(9) });
	}

	// endregion
}}

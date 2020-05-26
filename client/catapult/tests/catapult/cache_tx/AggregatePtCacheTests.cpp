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

#include "catapult/cache_tx/AggregatePtCache.h"
#include "catapult/model/Cosignature.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/other/mocks/MockPtChangeSubscriber.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS AggregatePtCacheTests

	namespace {
		// region basic mocks

		class UnsupportedPtCacheModifier : public PtCacheModifier {
		public:
			size_t size() const override {
				CATAPULT_THROW_RUNTIME_ERROR("size - not supported in mock");
			}

			bool add(const model::DetachedTransactionInfo&) override {
				CATAPULT_THROW_RUNTIME_ERROR("add - not supported in mock");
			}

			model::DetachedTransactionInfo add(const Hash256&, const model::Cosignature&) override {
				CATAPULT_THROW_RUNTIME_ERROR("add(cosignature) - not supported in mock");
			}

			model::DetachedTransactionInfo remove(const Hash256&) override {
				CATAPULT_THROW_RUNTIME_ERROR("remove - not supported in mock");
			}

			std::vector<model::DetachedTransactionInfo> prune(Timestamp) override {
				CATAPULT_THROW_RUNTIME_ERROR("prune - not supported in mock");
			}

			std::vector<model::DetachedTransactionInfo> prune(const predicate<const Hash256&>&) override {
				CATAPULT_THROW_RUNTIME_ERROR("prune - not supported in mock");
			}
		};

		template<typename TPtCacheModifier>
		using MockPtCache = test::MockTransactionsCache<PtCache, TPtCacheModifier, PtCacheModifierProxy>;

		// endregion

		struct PtTraits {
			using CacheType = PtCache;
			using ChangeSubscriberType = mocks::MockPtChangeSubscriber;
			using UnsupportedChangeSubscriberType = test::UnsupportedPtChangeSubscriber<test::UnsupportedFlushBehavior::Throw>;

			template<typename TModifier>
			using MockCacheType = MockPtCache<TModifier>;

			static constexpr auto CreateAggregateCache = CreateAggregatePtCache;
		};

		template<typename TModifier>
		using TestContext = test::TransactionsCacheTestContext<MockPtCache<TModifier>, PtTraits>;

		model::TransactionInfo StripMerkle(const model::TransactionInfo& transactionInfo) {
			// subscriber receives a TransactionInfo with a zeroed out merkle component hash
			// (because PtCache does not support merkle component hashes)
			auto infoCopy = transactionInfo.copy();
			infoCopy.MerkleComponentHash = Hash256();
			return infoCopy;
		}

		struct BasicTestsPtTraits {
		public:
			using CacheTraitsType = PtTraits;
			using UnsupportedCacheModifierType = UnsupportedPtCacheModifier;
			using TransactionInfoType = model::DetachedTransactionInfo;

			template<typename TModifier>
			using TestContextType = TestContext<TModifier>;

		public:
			static mocks::PtFlushInfo CreateFlushInfo(size_t numAdds, size_t numRemoves) {
				return { numAdds, 0, numRemoves };
			}

			static TransactionInfoType Copy(const TransactionInfoType& info) {
				return info.copy();
			}

			static model::TransactionInfo ToSubscriberInfo(const model::TransactionInfo& transactionInfo) {
				return StripMerkle(transactionInfo);
			}
		};
	}

	// region basic tests (add / remove / flush)

	DEFINE_AGGREGATE_TRANSACTIONS_CACHE_TESTS(TEST_CLASS, BasicTestsPtTraits)

	// endregion

	// region add(cosignature)

	namespace {
		class MockAddCosignaturePtCacheModifier : public UnsupportedPtCacheModifier {
		public:
			using CosignatureInfo = std::pair<Hash256, model::Cosignature>;

		public:
			MockAddCosignaturePtCacheModifier(
					std::vector<CosignatureInfo>& cosignatureInfos,
					const model::DetachedTransactionInfo& transactionInfo)
					: m_cosignatureInfos(cosignatureInfos)
					, m_transactionInfo(transactionInfo.copy())
			{}

		public:
			model::DetachedTransactionInfo add(const Hash256& parentHash, const model::Cosignature& cosignature) override {
				m_cosignatureInfos.emplace_back(parentHash, cosignature);
				return m_transactionInfo.copy();
			}

		private:
			std::vector<CosignatureInfo>& m_cosignatureInfos;
			model::DetachedTransactionInfo m_transactionInfo;
		};
	}

	TEST(TEST_CLASS, AddCosignatureDelegatesToCacheAndSubscriberOnCacheSuccess) {
		// Arrange:
		std::vector<MockAddCosignaturePtCacheModifier::CosignatureInfo> cosignatureInfos;
		auto transactionInfo = test::CreateRandomTransactionInfo();
		TestContext<MockAddCosignaturePtCacheModifier> context(cosignatureInfos, transactionInfo);

		auto parentHash = test::GenerateRandomByteArray<Hash256>();
		auto cosignature = test::CreateRandomDetachedCosignature();

		// Act: add via modifier, which flushes when destroyed
		auto transactionInfoFromAdd = context.aggregate().modifier().add(parentHash, cosignature);

		// Assert:
		test::AssertEqual(transactionInfo, transactionInfoFromAdd, "info from add");

		// - check pt cache modifier was called as expected
		ASSERT_EQ(1u, cosignatureInfos.size());
		EXPECT_EQ(parentHash, cosignatureInfos[0].first);
		EXPECT_EQ(cosignature.SignerPublicKey, cosignatureInfos[0].second.SignerPublicKey);
		EXPECT_EQ(cosignature.Signature, cosignatureInfos[0].second.Signature);

		// - check subscriber
		ASSERT_EQ(1u, context.subscriber().addedCosignatureInfos().size());
		const auto& addedCosignatureInfo = context.subscriber().addedCosignatureInfos()[0];
		test::AssertEqual(StripMerkle(transactionInfo), *addedCosignatureInfo.first, "info from subscriber");
		EXPECT_EQ(cosignature.SignerPublicKey, addedCosignatureInfo.second.SignerPublicKey);
		EXPECT_EQ(cosignature.Signature, addedCosignatureInfo.second.Signature);

		ASSERT_EQ(1u, context.subscriber().flushInfos().size());
		EXPECT_EQ(mocks::PtFlushInfo({ 0u, 1u, 0u }), context.subscriber().flushInfos()[0]);
	}

	TEST(TEST_CLASS, AddCosignatureDelegatesToCacheOnlyOnCacheFailure) {
		// Arrange:
		std::vector<MockAddCosignaturePtCacheModifier::CosignatureInfo> cosignatureInfos;
		TestContext<MockAddCosignaturePtCacheModifier> context(cosignatureInfos, model::TransactionInfo());

		auto parentHash = test::GenerateRandomByteArray<Hash256>();
		auto cosignature = test::CreateRandomDetachedCosignature();

		// Act: add via modifier, which flushes when destroyed
		auto transactionInfoFromAdd = context.aggregate().modifier().add(parentHash, cosignature);

		// Assert:
		EXPECT_FALSE(!!transactionInfoFromAdd);

		// - check pt cache modifier was called as expected
		ASSERT_EQ(1u, cosignatureInfos.size());
		EXPECT_EQ(parentHash, cosignatureInfos[0].first);
		test::AssertCosignature(cosignature, cosignatureInfos[0].second);

		// - check subscriber
		ASSERT_EQ(1u, context.subscriber().flushInfos().size());
		EXPECT_EQ(mocks::PtFlushInfo({ 0u, 0u, 0u }), context.subscriber().flushInfos()[0]);
	}

	// endregion

	// region prune (timestamp)

	namespace {
		class MockPruneTimestampPtCacheModifier : public UnsupportedPtCacheModifier {
		public:
			MockPruneTimestampPtCacheModifier(
					std::vector<Timestamp>& timestamps,
					std::vector<model::DetachedTransactionInfo>&& transactionInfos)
					: m_timestamps(timestamps)
					, m_transactionInfos(std::move(transactionInfos))
			{}

		public:
			std::vector<model::DetachedTransactionInfo> prune(Timestamp timestamp) override {
				m_timestamps.push_back(timestamp);
				return std::move(m_transactionInfos);
			}

		private:
			std::vector<Timestamp>& m_timestamps;
			std::vector<model::DetachedTransactionInfo> m_transactionInfos;
		};

		std::vector<model::DetachedTransactionInfo> ToDetachedTransactionInfos(
				const std::vector<model::TransactionInfo>& transactionInfosWithMerkleHashes) {
			std::vector<model::DetachedTransactionInfo> transactionInfos;
			for (const auto& transactionInfo : transactionInfosWithMerkleHashes)
				transactionInfos.push_back(transactionInfo.copy());

			return transactionInfos;
		}

		std::vector<model::TransactionInfo> StripMerkles(const std::vector<model::TransactionInfo>& transactionInfosWithMerkleHashes) {
			std::vector<model::TransactionInfo> transactionInfos;
			for (auto& transactionInfo : transactionInfosWithMerkleHashes)
				transactionInfos.push_back(StripMerkle(transactionInfo));

			return transactionInfos;
		}
	}

	TEST(TEST_CLASS, PruneTimestampDelegatesToCacheOnlyWhenCacheIsEmpty) {
		// Arrange:
		std::vector<Timestamp> pruneTimestamps;
		TestContext<MockPruneTimestampPtCacheModifier> context(pruneTimestamps, std::vector<model::DetachedTransactionInfo>());

		// Act:
		auto prunedInfos = context.aggregate().modifier().prune(Timestamp(123));

		// Assert:
		EXPECT_TRUE(prunedInfos.empty());

		// - check pt cache modifier was called as expected
		ASSERT_EQ(1u, pruneTimestamps.size());
		EXPECT_EQ(Timestamp(123), pruneTimestamps[0]);

		// - check subscriber
		ASSERT_EQ(1u, context.subscriber().flushInfos().size());
		EXPECT_EQ(mocks::PtFlushInfo({ 0u, 0u, 0u }), context.subscriber().flushInfos()[0]);
	}

	TEST(TEST_CLASS, PruneTimestampDelegatesToCacheAndSubscriberWhenCacheIsNotEmpty) {
		// Arrange:
		std::vector<Timestamp> pruneTimestamps;
		auto transactionInfos = test::CreateTransactionInfos(5);
		auto transactionInfosWithoutMerkleHashes = ToDetachedTransactionInfos(transactionInfos);
		TestContext<MockPruneTimestampPtCacheModifier> context(pruneTimestamps, ToDetachedTransactionInfos(transactionInfos));

		// Act:
		auto prunedInfos = context.aggregate().modifier().prune(Timestamp(123));

		// Assert:
		ASSERT_EQ(5u, prunedInfos.size());
		for (auto i = 0u; i < transactionInfosWithoutMerkleHashes.size(); ++i)
			test::AssertEqual(transactionInfosWithoutMerkleHashes[i], prunedInfos[i], "info from prune " + std::to_string(i));

		// - check pt cache modifier was called as expected
		ASSERT_EQ(1u, pruneTimestamps.size());
		EXPECT_EQ(Timestamp(123), pruneTimestamps[0]);

		// - check subscriber
		ASSERT_EQ(5u, context.subscriber().removedInfos().size());
		test::AssertEquivalent(StripMerkles(transactionInfos), context.subscriber().removedInfos(), "subscriber infos");

		ASSERT_EQ(1u, context.subscriber().flushInfos().size());
		EXPECT_EQ(mocks::PtFlushInfo({ 0u, 0u, 5u }), context.subscriber().flushInfos()[0]);
	}

	// endregion

	// region prune (predicate)

	namespace {
		class MockPrunePredicatePtCacheModifier : public UnsupportedPtCacheModifier {
		public:
			explicit MockPrunePredicatePtCacheModifier(std::vector<model::DetachedTransactionInfo>&& transactionInfos)
					: m_transactionInfos(std::move(transactionInfos))
			{}

		public:
			std::vector<model::DetachedTransactionInfo> prune(const predicate<const Hash256&>& hashPredicate) override {
				hashPredicate(Hash256());
				return std::move(m_transactionInfos);
			}

		private:
			std::vector<model::DetachedTransactionInfo> m_transactionInfos;
		};
	}

	TEST(TEST_CLASS, PrunePredicateDelegatesToCacheOnlyWhenCacheIsEmpty) {
		// Arrange:
		auto transactionInfos = std::vector<model::DetachedTransactionInfo>();
		TestContext<MockPrunePredicatePtCacheModifier> context(std::move(transactionInfos));
		auto numPredicateCalls = 0u;

		// Act:
		auto prunedInfos = context.aggregate().modifier().prune([&numPredicateCalls](const auto&) {
			++numPredicateCalls;
			return true;
		});

		// Assert:
		EXPECT_TRUE(prunedInfos.empty());

		// - check pt cache modifier was called as expected
		EXPECT_EQ(1u, numPredicateCalls);

		// - check subscriber
		ASSERT_EQ(1u, context.subscriber().flushInfos().size());
		EXPECT_EQ(mocks::PtFlushInfo({ 0u, 0u, 0u }), context.subscriber().flushInfos()[0]);
	}

	TEST(TEST_CLASS, PrunePredicateDelegatesToCacheAndSubscriberWhenCacheIsNotEmpty) {
		// Arrange:
		auto transactionInfos = test::CreateTransactionInfos(5);
		auto transactionInfosWithoutMerkleHashes = ToDetachedTransactionInfos(transactionInfos);
		TestContext<MockPrunePredicatePtCacheModifier> context(ToDetachedTransactionInfos(transactionInfos));
		auto numPredicateCalls = 0u;

		// Act:
		auto prunedInfos = context.aggregate().modifier().prune([&numPredicateCalls](const auto&) {
			++numPredicateCalls;
			return true;
		});

		// Assert:
		ASSERT_EQ(5u, prunedInfos.size());
		for (auto i = 0u; i < transactionInfosWithoutMerkleHashes.size(); ++i)
			test::AssertEqual(transactionInfosWithoutMerkleHashes[i], prunedInfos[i], "info from prune " + std::to_string(i));

		// - check pt cache modifier was called as expected
		EXPECT_EQ(1u, numPredicateCalls);

		// - check subscriber
		ASSERT_EQ(5u, context.subscriber().removedInfos().size());
		test::AssertEquivalent(StripMerkles(transactionInfos), context.subscriber().removedInfos(), "subscriber infos");

		ASSERT_EQ(1u, context.subscriber().flushInfos().size());
		EXPECT_EQ(mocks::PtFlushInfo({ 0u, 0u, 5u }), context.subscriber().flushInfos()[0]);
	}

	// endregion
}}

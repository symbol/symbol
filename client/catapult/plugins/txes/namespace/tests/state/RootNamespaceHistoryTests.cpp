#include "src/state/RootNamespaceHistory.h"
#include "tests/test/NamespaceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

	namespace {
		auto CreateDefaultChildren() {
			return test::CreateChildren({
				test::CreatePath({ 123, 357 }),
				test::CreatePath({ 123, 124 }),
				test::CreatePath({ 123, 124, 125 }),
				test::CreatePath({ 123, 124, 128 })
			});
		}

		void AddDefaultChildren(RootNamespace& root) {
			auto children = CreateDefaultChildren();
			test::AddAll(root, children, { 357, 124, 125, 128 });
		}

		auto CreateDefaultHistory() {
			auto owner = test::CreateRandomOwner();
			auto diffOwner = test::CreateRandomOwner();

			RootNamespaceHistory history(NamespaceId(123));
			history.push_back(owner, test::CreateLifetime(234, 321));
			history.push_back(diffOwner, test::CreateLifetime(355, 635));
			history.push_back(owner, test::CreateLifetime(567, 689));

			// Sanity:
			EXPECT_EQ(3u, history.historyDepth());
			return history;
		}

		std::vector<Height> GetLifetimeStartHeights(const RootNamespaceHistory& history) {
			// check the start heights as a way to validate iteration
			std::vector<Height> heights;
			for (auto iter = history.cbegin(); history.cend() != iter; ++iter)
				heights.push_back(iter->lifetime().Start);

			return heights;
		}
	}

	TEST(RootNamespaceHistoryTests, CanCreateEmptyHistory) {
		// Act:
		RootNamespaceHistory history(NamespaceId(123));

		// Assert:
		EXPECT_EQ(NamespaceId(123), history.id());
		EXPECT_TRUE(history.empty());
		EXPECT_EQ(0u, history.historyDepth());
		EXPECT_EQ(0u, history.numActiveRootChildren());
		EXPECT_EQ(0u, history.numAllHistoricalChildren());
		EXPECT_EQ(std::vector<Height>(), GetLifetimeStartHeights(history));
	}

	// region push_back

	TEST(RootNamespaceHistoryTests, CanAddHistory) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		RootNamespaceHistory history(NamespaceId(123));

		// Act:
		history.push_back(owner, test::CreateLifetime(234, 321));

		// Assert:
		EXPECT_EQ(NamespaceId(123), history.id());
		EXPECT_FALSE(history.empty());
		EXPECT_EQ(1u, history.historyDepth());
		EXPECT_EQ(0u, history.numActiveRootChildren());
		EXPECT_EQ(0u, history.numAllHistoricalChildren());
		EXPECT_EQ(std::vector<Height> ({ Height(234) }), GetLifetimeStartHeights(history));
	}

	TEST(RootNamespaceHistoryTests, CanAddRootNamespaceWithSameOwnerToExistingHistory) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		RootNamespaceHistory history(NamespaceId(123));
		history.push_back(owner, test::CreateLifetime(234, 321));

		// Act:
		history.push_back(owner, test::CreateLifetime(355, 469));

		// Assert:
		EXPECT_EQ(NamespaceId(123), history.id());
		EXPECT_FALSE(history.empty());
		EXPECT_EQ(2u, history.historyDepth());
		EXPECT_EQ(0u, history.numActiveRootChildren());
		EXPECT_EQ(0u, history.numAllHistoricalChildren());
		EXPECT_EQ(std::vector<Height> ({ Height(234), Height(355) }), GetLifetimeStartHeights(history));
	}

	TEST(RootNamespaceHistoryTests, CanAddRootNamespaceWithDifferentOwnerToExistingHistory) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		RootNamespaceHistory history(NamespaceId(123));
		history.push_back(owner, test::CreateLifetime(234, 321));

		// Act:
		auto diffOwner = test::CreateRandomOwner();
		history.push_back(diffOwner, test::CreateLifetime(355, 469));

		// Assert:
		EXPECT_EQ(NamespaceId(123), history.id());
		EXPECT_FALSE(history.empty());
		EXPECT_EQ(2u, history.historyDepth());
		EXPECT_EQ(0u, history.numActiveRootChildren());
		EXPECT_EQ(0u, history.numAllHistoricalChildren());
		EXPECT_EQ(std::vector<Height> ({ Height(234), Height(355) }), GetLifetimeStartHeights(history));
	}

	TEST(RootNamespaceHistoryTests, AddingRootWithSameOwnerSharesChildrenWithNewRoot) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		RootNamespaceHistory history(NamespaceId(123));
		history.push_back(owner, test::CreateLifetime(234, 321));
		auto& originalRoot = history.back();
		AddDefaultChildren(originalRoot);

		// Sanity:
		EXPECT_EQ(4u, history.numActiveRootChildren());
		EXPECT_EQ(4u, history.numAllHistoricalChildren());

		// Act:
		history.push_back(owner, test::CreateLifetime(355, 469));
		const auto& secondRoot = history.back();
		history.push_back(owner, test::CreateLifetime(579, 876));

		// Assert:
		const auto& thirdRoot = history.back();
		EXPECT_EQ(NamespaceId(123), history.id());
		EXPECT_FALSE(history.empty());
		EXPECT_EQ(3u, history.historyDepth());
		EXPECT_EQ(4u, history.numActiveRootChildren());
		EXPECT_EQ(12u, history.numAllHistoricalChildren());
		EXPECT_EQ(std::vector<Height> ({ Height(234), Height(355), Height(579) }), GetLifetimeStartHeights(history));

		// - all roots have the same owner and share children
		EXPECT_EQ(4u, thirdRoot.children().size());
		test::AssertChildren(CreateDefaultChildren(), thirdRoot.children());
		EXPECT_EQ(4u, secondRoot.children().size());
		test::AssertChildren(CreateDefaultChildren(), secondRoot.children());
		EXPECT_EQ(4u, originalRoot.children().size());
		test::AssertChildren(CreateDefaultChildren(), originalRoot.children());
	}

	TEST(RootNamespaceHistoryTests, AddingRootWithDifferentOwnerDoesNotShareChildrenWithNewRoot) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		RootNamespaceHistory history(NamespaceId(123));
		history.push_back(owner, test::CreateLifetime(234, 321));
		auto& originalRoot = history.back();
		AddDefaultChildren(originalRoot);

		// Sanity:
		EXPECT_EQ(4u, history.numActiveRootChildren());
		EXPECT_EQ(4u, history.numAllHistoricalChildren());

		// Act:
		auto diffOwner = test::CreateRandomOwner();
		history.push_back(diffOwner, test::CreateLifetime(355, 469));

		// Assert:
		const auto& secondRoot = history.back();
		EXPECT_EQ(NamespaceId(123), history.id());
		EXPECT_FALSE(history.empty());
		EXPECT_EQ(2u, history.historyDepth());
		EXPECT_EQ(0u, history.numActiveRootChildren());
		EXPECT_EQ(4u, history.numAllHistoricalChildren());
		EXPECT_EQ(std::vector<Height> ({ Height(234), Height(355) }), GetLifetimeStartHeights(history));

		EXPECT_EQ(0u, secondRoot.children().size());
		EXPECT_EQ(4u, originalRoot.children().size());
		test::AssertChildren(CreateDefaultChildren(), originalRoot.children());
	}

	TEST(RootNamespaceHistoryTests, AddingRootWithOriginalOwnerDoesNotShareChildrenIfRootWithDifferentOwnerIsInbetween) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		RootNamespaceHistory history(NamespaceId(123));
		history.push_back(owner, test::CreateLifetime(234, 321));
		auto& originalRoot = history.back();
		AddDefaultChildren(originalRoot);
		auto diffOwner = test::CreateRandomOwner();
		history.push_back(diffOwner, test::CreateLifetime(355, 469));
		const auto& secondRoot = history.back();

		// Sanity:
		EXPECT_EQ(0u, history.numActiveRootChildren());
		EXPECT_EQ(4u, history.numAllHistoricalChildren());

		// Act: add root with same owner as original root
		history.push_back(owner, test::CreateLifetime(579, 876));

		// Assert:
		const auto& thirdRoot = history.back();
		EXPECT_EQ(NamespaceId(123), history.id());
		EXPECT_FALSE(history.empty());
		EXPECT_EQ(3u, history.historyDepth());
		EXPECT_EQ(0u, history.numActiveRootChildren());
		EXPECT_EQ(4u, history.numAllHistoricalChildren());
		EXPECT_EQ(std::vector<Height> ({ Height(234), Height(355), Height(579) }), GetLifetimeStartHeights(history));

		EXPECT_EQ(4u, originalRoot.children().size());
		test::AssertChildren(CreateDefaultChildren(), originalRoot.children());

		// neither second nor third root has inherited any children
		EXPECT_EQ(0u, secondRoot.children().size());
		EXPECT_EQ(0u, thirdRoot.children().size());
	}

	// endregion

	// region pop_back

	TEST(RootNamespaceHistoryTests, PopBackRemovesLastRoot) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		auto diffOwner = test::CreateRandomOwner();

		RootNamespaceHistory history(NamespaceId(123));
		history.push_back(owner, test::CreateLifetime(234, 321));
		history.push_back(diffOwner, test::CreateLifetime(355, 635));
		auto& root = history.back();
		history.push_back(owner, test::CreateLifetime(567, 689));

		// Sanity:
		EXPECT_EQ(3u, history.historyDepth());

		// Act:
		history.pop_back();

		// Assert:
		EXPECT_EQ(NamespaceId(123), history.id());
		EXPECT_EQ(2u, history.historyDepth());
		EXPECT_EQ(&root, &history.back());

		EXPECT_EQ(0u, history.numActiveRootChildren());
		EXPECT_EQ(0u, history.numAllHistoricalChildren());
		EXPECT_EQ(std::vector<Height> ({ Height(234), Height(355) }), GetLifetimeStartHeights(history));
	}

	// endregion

	// region back

	TEST(RootNamespaceHistoryTests, BackReturnsMostRecentRoot) {
		// Arrange:
		auto history = CreateDefaultHistory();

		// Act:
		auto& root = history.back();
		auto& constRoot = const_cast<const RootNamespaceHistory&>(history).back();

		// Assert:
		EXPECT_EQ(NamespaceId(123), history.id());
		EXPECT_FALSE(std::is_const<std::remove_reference<decltype(root)>::type>::value);
		EXPECT_EQ(&constRoot, &root);
		EXPECT_EQ(test::CreateLifetime(567, 689), root.lifetime());
		EXPECT_EQ(test::CreateLifetime(567, 689), constRoot.lifetime());
	}

	// endregion

	// region prune

	namespace {
		void AssertPrunedNamespaceIds(
				Height::ValueType pruneHeight,
				size_t expectedHistoryDepth,
				std::initializer_list<NamespaceId::ValueType> expectedIds) {
			auto owner = test::CreateRandomOwner();
			auto diffOwner = test::CreateRandomOwner();

			RootNamespaceHistory history(NamespaceId(123));
			history.push_back(owner, test::CreateLifetime(234, 321));
			AddDefaultChildren(history.back()); // child ids 357, 124, 125, 128
			history.push_back(owner, test::CreateLifetime(355, 635)); // renewal
			auto& secondRoot = history.back();
			secondRoot.add(Namespace(test::CreatePath({ 123, 298 })));
			history.push_back(diffOwner, test::CreateLifetime(567, 689));

			// Act:
			auto ids = history.prune(Height(pruneHeight));

			// Assert:
			if (pruneHeight >= 689) {
				// all roots were pruned
				EXPECT_TRUE(history.empty());
			} else {
				// not all roots were pruned
				const auto& root = history.back();
				EXPECT_EQ(test::CreateLifetime(567, 689), root.lifetime());
			}

			EXPECT_EQ(expectedHistoryDepth, history.historyDepth());
			EXPECT_EQ(expectedHistoryDepth, GetLifetimeStartHeights(history).size());

			EXPECT_EQ(expectedIds.size(), ids.size());
			for (auto id : expectedIds)
				EXPECT_EQ(1u, ids.count(NamespaceId(id))) << "for id " << id;
		}
	}

	TEST(RootNamespaceHistoryTests, PruneRespectsRenewalOfNamespace) {
		// Assert:
		// - although the first root in the list has expired, the second root prevents the children from being pruned
		//   because it renewed the owner's namespace
		// - the most recent root is not yet active (lifetime begins at height 567), still prune will not touch it
		AssertPrunedNamespaceIds(431, 2u, {});
	}

	TEST(RootNamespaceHistoryTests, PruneReturnsNamespaceIdsOfAllExpiredChildren) {
		// Assert: all children should be removed but the root id itself should stay
		std::initializer_list<NamespaceId::ValueType> ids{ 357u, 124u, 125u, 128u, 298u };
		AssertPrunedNamespaceIds(655, 1u, ids);
		AssertPrunedNamespaceIds(688, 1u, ids);
	}

	TEST(RootNamespaceHistoryTests, PruneReturnsNamespaceIdsIncludingRootIdIfAllEntriesWerePruned) {
		// Assert: all roots have expired, even root id should be removed
		std::initializer_list<NamespaceId::ValueType> ids{ 123u, 357u, 124u, 125u, 128u, 298u };
		AssertPrunedNamespaceIds(689, 0u, ids);
		AssertPrunedNamespaceIds(700, 0u, ids);
		AssertPrunedNamespaceIds(10000, 0u, ids);
	}

	TEST(RootNamespaceHistoryTests, PruneCanBeCalledOnEmptyHistory) {
		// Arrange:
		RootNamespaceHistory history(NamespaceId(123));

		// Sanity:
		EXPECT_TRUE(history.empty());

		// Act:
		history.prune(Height(234));

		// Assert:
		EXPECT_TRUE(history.empty());
	}

	// endregion
}}

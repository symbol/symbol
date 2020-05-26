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

#include "src/state/RootNamespaceHistory.h"
#include "tests/test/NamespaceTestUtils.h"
#include "tests/TestHarness.h"
#include <unordered_set>

namespace catapult { namespace state {

#define TEST_CLASS RootNamespaceHistoryTests

	namespace {
		using IdSet = std::unordered_set<NamespaceId, utils::BaseValueHasher<NamespaceId>>;

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

		std::vector<Address> GetOwners(const RootNamespaceHistory& history) {
			std::vector<Address> owners;
			for (const auto& root : history)
				owners.push_back(root.ownerAddress());

			return owners;
		}

		std::vector<Height> GetLifetimeStartHeights(const RootNamespaceHistory& history) {
			// check the start heights as a way to validate iteration
			std::vector<Height> heights;
			for (const auto& root : history)
				heights.push_back(root.lifetime().Start);

			return heights;
		}

		std::vector<IdSet> GetChildIdSets(const RootNamespaceHistory& history) {
			std::vector<IdSet> idSets;
			for (const auto& root : history) {
				IdSet ids;
				const auto& children = root.children();
				for (const auto& pair : children)
					ids.insert(pair.first);

				idSets.push_back(ids);
			}

			return idSets;
		}
	}

	// region constructor

	namespace {
		struct CopyConstructorTraits {
			static RootNamespaceHistory Construct(const RootNamespaceHistory& history) {
				return RootNamespaceHistory(history);
			}
		};

		struct MoveConstructorTraits {
			static RootNamespaceHistory Construct(RootNamespaceHistory&& history) {
				return RootNamespaceHistory(std::move(history));
			}
		};
	}

#define CONSTRUCTOR_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_CopyConstructor) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CopyConstructorTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MoveConstructor) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MoveConstructorTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TEST(TEST_CLASS, CanCreateEmptyHistory) {
		// Act:
		RootNamespaceHistory history(NamespaceId(123));

		// Assert:
		EXPECT_EQ(NamespaceId(123), history.id());
		EXPECT_TRUE(history.empty());
		EXPECT_EQ(0u, history.historyDepth());
		EXPECT_EQ(0u, history.activeOwnerHistoryDepth());
		EXPECT_EQ(0u, history.numActiveRootChildren());
		EXPECT_EQ(0u, history.numAllHistoricalChildren());
		EXPECT_EQ(std::vector<Height>(), GetLifetimeStartHeights(history));
	}

	CONSTRUCTOR_TEST(CanConstructEmptyHistory) {
		// Arrange:
		RootNamespaceHistory original(NamespaceId(123));

		// Act:
		auto history = TTraits::Construct(std::move(original));

		// Assert:
		EXPECT_EQ(NamespaceId(123), history.id());
		EXPECT_TRUE(history.empty());
		EXPECT_EQ(0u, history.historyDepth());
		EXPECT_EQ(0u, history.activeOwnerHistoryDepth());
		EXPECT_EQ(0u, history.numActiveRootChildren());
		EXPECT_EQ(0u, history.numAllHistoricalChildren());
		EXPECT_EQ(std::vector<Height>(), GetLifetimeStartHeights(history));
	}

	namespace {
		template<typename TTraits, typename TAliasGenerator, typename TAliasChecker>
		void AssertCanConstructHistoryWithSingleOwnerWithoutChildren(TAliasGenerator aliasGenerator, TAliasChecker aliasChecker) {
			// Arrange:
			auto owner = test::CreateRandomOwner();
			RootNamespaceHistory original(NamespaceId(123));
			for (auto i = 0u; i < 3; ++i) {
				original.push_back(owner, test::CreateLifetime(234 + i, 321 + i));
				original.back().setAlias(NamespaceId(123), aliasGenerator(i));
			}

			// Act:
			auto history = TTraits::Construct(std::move(original));

			// Assert:
			EXPECT_EQ(NamespaceId(123), history.id());
			EXPECT_FALSE(history.empty());
			EXPECT_EQ(3u, history.historyDepth());
			EXPECT_EQ(3u, history.activeOwnerHistoryDepth());
			EXPECT_EQ(0u, history.numActiveRootChildren());
			EXPECT_EQ(0u, history.numAllHistoricalChildren());
			EXPECT_EQ(std::vector<Address>({ owner, owner, owner }), GetOwners(history));
			EXPECT_EQ(std::vector<Height>({ Height(234), Height(235), Height(236) }), GetLifetimeStartHeights(history));

			// - check (root) aliases
			auto i = 0u;
			for (const auto& root : history) {
				aliasChecker(root.alias(NamespaceId(123)), i, "root at " + std::to_string(i));
				++i;
			}
		}
	}

	CONSTRUCTOR_TEST(CanConstructHistoryWithSingleOwner_WithoutChildren) {
		AssertCanConstructHistoryWithSingleOwnerWithoutChildren<TTraits>(
				[](auto) { return NamespaceAlias(); },
				[](const auto& rootAlias, auto, const auto& message) {
					EXPECT_EQ(AliasType::None, rootAlias.type()) << message;
				});
	}

	CONSTRUCTOR_TEST(CanConstructHistoryWithSingleOwner_WithoutChildren_WithAlias) {
		AssertCanConstructHistoryWithSingleOwnerWithoutChildren<TTraits>(
				[](auto i) { return NamespaceAlias(MosaicId(999 + 2 * i)); },
				[](const auto& rootAlias, auto i, const auto& message) {
					ASSERT_EQ(AliasType::Mosaic, rootAlias.type()) << message;
					EXPECT_EQ(MosaicId(999 + 2 * i), rootAlias.mosaicId()) << message;
				});
	}

	CONSTRUCTOR_TEST(CanConstructHistoryWithSingleOwner_WithChildren) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		RootNamespaceHistory original(NamespaceId(123));
		for (auto i = 0u; i < 3; ++i) {
			original.push_back(owner, test::CreateLifetime(234 + i, 321 + i));
			original.back().add(Namespace(test::CreatePath({ 123, 100 + i })));
		}

		// Act:
		auto history = TTraits::Construct(std::move(original));

		// Assert:
		std::vector<IdSet> expectedChildIds{
			{ NamespaceId(100), NamespaceId(101), NamespaceId(102) },
			{ NamespaceId(100), NamespaceId(101), NamespaceId(102) },
			{ NamespaceId(100), NamespaceId(101), NamespaceId(102) }
		};
		EXPECT_EQ(NamespaceId(123), history.id());
		EXPECT_FALSE(history.empty());
		EXPECT_EQ(3u, history.historyDepth());
		EXPECT_EQ(3u, history.activeOwnerHistoryDepth());
		EXPECT_EQ(3u, history.numActiveRootChildren());
		EXPECT_EQ(9u, history.numAllHistoricalChildren());
		EXPECT_EQ(std::vector<Address>({ owner, owner, owner }), GetOwners(history));
		EXPECT_EQ(std::vector<Height>({ Height(234), Height(235), Height(236) }), GetLifetimeStartHeights(history));
		EXPECT_EQ(expectedChildIds, GetChildIdSets(history));
	}

	CONSTRUCTOR_TEST(CanConstructHistoryWithDifferentOwners_WithoutChildren) {
		// Arrange:
		auto owners = test::GenerateRandomDataVector<Address>(2);
		RootNamespaceHistory original(NamespaceId(123));
		for (auto i = 0u; i < 3; ++i)
			original.push_back(owners[i % 2], test::CreateLifetime(234 + i, 321 + i));

		// Act:
		auto history = TTraits::Construct(std::move(original));

		// Assert:
		EXPECT_EQ(NamespaceId(123), history.id());
		EXPECT_FALSE(history.empty());
		EXPECT_EQ(3u, history.historyDepth());
		EXPECT_EQ(1u, history.activeOwnerHistoryDepth());
		EXPECT_EQ(0u, history.numActiveRootChildren());
		EXPECT_EQ(0u, history.numAllHistoricalChildren());
		EXPECT_EQ(std::vector<Address>({ owners[0], owners[1], owners[0] }), GetOwners(history));
		EXPECT_EQ(std::vector<Height>({ Height(234), Height(235), Height(236) }), GetLifetimeStartHeights(history));
	}

	CONSTRUCTOR_TEST(CanConstructHistoryWithDifferentOwners_WithChildren) {
		// Arrange:
		auto owners = test::GenerateRandomDataVector<Address>(2);
		RootNamespaceHistory original(NamespaceId(123));

		// - let the history have 2 roots that share the owner before the owner changes
		original.push_back(owners[0], test::CreateLifetime(100, 321));
		original.back().add(Namespace(test::CreatePath({ 123, 99 })));
		for (auto i = 0u; i < 3; ++i) {
			original.push_back(owners[i % 2], test::CreateLifetime(234 + i, 321 + i));
			original.back().add(Namespace(test::CreatePath({ 123, 100 + i })));
		}

		// Act:
		auto history = TTraits::Construct(std::move(original));

		// Assert:
		std::vector<IdSet> expectedChildIds{
			{ NamespaceId(99), NamespaceId(100) },
			{ NamespaceId(99), NamespaceId(100) },
			{ NamespaceId(101) },
			{ NamespaceId(102) }
		};
		EXPECT_EQ(NamespaceId(123), history.id());
		EXPECT_FALSE(history.empty());
		EXPECT_EQ(4u, history.historyDepth());
		EXPECT_EQ(1u, history.activeOwnerHistoryDepth());
		EXPECT_EQ(1u, history.numActiveRootChildren());
		EXPECT_EQ(6u, history.numAllHistoricalChildren());
		EXPECT_EQ(std::vector<Address>({ owners[0], owners[0], owners[1], owners[0] }), GetOwners(history));
		EXPECT_EQ(std::vector<Height>({ Height(100), Height(234), Height(235), Height(236) }), GetLifetimeStartHeights(history));
		EXPECT_EQ(expectedChildIds, GetChildIdSets(history));
	}

	// endregion

	// region push_back

	TEST(TEST_CLASS, CanAddHistory) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		RootNamespaceHistory history(NamespaceId(123));

		// Act:
		history.push_back(owner, test::CreateLifetime(234, 321));

		// Assert:
		EXPECT_EQ(NamespaceId(123), history.id());
		EXPECT_FALSE(history.empty());
		EXPECT_EQ(1u, history.historyDepth());
		EXPECT_EQ(1u, history.activeOwnerHistoryDepth());
		EXPECT_EQ(0u, history.numActiveRootChildren());
		EXPECT_EQ(0u, history.numAllHistoricalChildren());
		EXPECT_EQ(std::vector<Height> ({ Height(234) }), GetLifetimeStartHeights(history));
	}

	TEST(TEST_CLASS, CanAddRootNamespaceWithSameOwnerToExistingHistory) {
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
		EXPECT_EQ(2u, history.activeOwnerHistoryDepth());
		EXPECT_EQ(0u, history.numActiveRootChildren());
		EXPECT_EQ(0u, history.numAllHistoricalChildren());
		EXPECT_EQ(std::vector<Height> ({ Height(234), Height(355) }), GetLifetimeStartHeights(history));
	}

	TEST(TEST_CLASS, CanAddRootNamespaceWithDifferentOwnerToExistingHistory) {
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
		EXPECT_EQ(1u, history.activeOwnerHistoryDepth());
		EXPECT_EQ(0u, history.numActiveRootChildren());
		EXPECT_EQ(0u, history.numAllHistoricalChildren());
		EXPECT_EQ(std::vector<Height> ({ Height(234), Height(355) }), GetLifetimeStartHeights(history));
	}

	TEST(TEST_CLASS, CanAddRootNamespaceWithSameOwnerToExistingHistoryWithAlias) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		RootNamespaceHistory history(NamespaceId(123));
		history.push_back(owner, test::CreateLifetime(234, 321));
		history.back().setAlias(NamespaceId(123), NamespaceAlias(MosaicId(999)));

		// Act:
		history.push_back(owner, test::CreateLifetime(355, 469));

		// Assert: alias is still set
		const auto& rootAlias = history.back().alias(NamespaceId(123));
		EXPECT_EQ(AliasType::Mosaic, rootAlias.type());
		EXPECT_EQ(MosaicId(999), rootAlias.mosaicId());
	}

	TEST(TEST_CLASS, CanAddRootNamespaceWithDifferentOwnerToExistingHistoryWithAlias) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		RootNamespaceHistory history(NamespaceId(123));
		history.push_back(owner, test::CreateLifetime(234, 321));
		history.back().setAlias(NamespaceId(123), NamespaceAlias(MosaicId(999)));

		// Act:
		auto diffOwner = test::CreateRandomOwner();
		history.push_back(diffOwner, test::CreateLifetime(355, 469));

		// Assert: alias is cleared
		const auto& rootAlias = history.back().alias(NamespaceId(123));
		EXPECT_EQ(AliasType::None, rootAlias.type());
	}

	TEST(TEST_CLASS, AddingRootWithSameOwnerSharesChildrenWithNewRoot) {
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
		EXPECT_EQ(3u, history.activeOwnerHistoryDepth());
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

	TEST(TEST_CLASS, AddingRootWithDifferentOwnerDoesNotShareChildrenWithNewRoot) {
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
		EXPECT_EQ(1u, history.activeOwnerHistoryDepth());
		EXPECT_EQ(0u, history.numActiveRootChildren());
		EXPECT_EQ(4u, history.numAllHistoricalChildren());
		EXPECT_EQ(std::vector<Height> ({ Height(234), Height(355) }), GetLifetimeStartHeights(history));

		EXPECT_EQ(0u, secondRoot.children().size());
		EXPECT_EQ(4u, originalRoot.children().size());
		test::AssertChildren(CreateDefaultChildren(), originalRoot.children());
	}

	TEST(TEST_CLASS, AddingRootWithOriginalOwnerDoesNotShareChildrenWhenRootWithDifferentOwnerIsInbetween) {
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
		EXPECT_EQ(1u, history.activeOwnerHistoryDepth());
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

	TEST(TEST_CLASS, PopBackRemovesLastRoot) {
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
		EXPECT_EQ(1u, history.activeOwnerHistoryDepth());
		EXPECT_EQ(&root, &history.back());

		EXPECT_EQ(0u, history.numActiveRootChildren());
		EXPECT_EQ(0u, history.numAllHistoricalChildren());
		EXPECT_EQ(std::vector<Height> ({ Height(234), Height(355) }), GetLifetimeStartHeights(history));
	}

	// endregion

	// region back

	TEST(TEST_CLASS, BackReturnsMostRecentRoot) {
		// Arrange:
		auto history = CreateDefaultHistory();

		// Act:
		auto& root = history.back();
		auto& constRoot = const_cast<const RootNamespaceHistory&>(history).back();

		// Assert:
		EXPECT_EQ(NamespaceId(123), history.id());
		EXPECT_FALSE(std::is_const_v<std::remove_reference_t<decltype(root)>>);
		EXPECT_EQ(&constRoot, &root);
		EXPECT_EQ(test::CreateLifetime(567, 689), root.lifetime());
		EXPECT_EQ(test::CreateLifetime(567, 689), constRoot.lifetime());
	}

	// endregion

	// region prune

	namespace {
		constexpr auto Grace_Period_Duration = 7u;

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

	TEST(TEST_CLASS, PruneRespectsRenewalOfNamespace) {
		// Assert: although the first root in the list has expired, the second root prevents the children from being pruned
		//   because it renewed the owner's namespace
		// - the most recent root is not yet active (lifetime begins at height 567), still prune will not touch it
		AssertPrunedNamespaceIds(431, 2u, {});
	}

	TEST(TEST_CLASS, PruneReturnsNamespaceIdsOfAllExpiredChildren) {
		// Assert: all children should be removed but the root id itself should stay
		std::initializer_list<NamespaceId::ValueType> ids{ 357u, 124u, 125u, 128u, 298u };
		AssertPrunedNamespaceIds(655, 1u, ids);
		AssertPrunedNamespaceIds(688, 1u, ids);
	}

	TEST(TEST_CLASS, PruneReturnsNamespaceIdsIncludingRootIdWhenAllEntriesWerePruned) {
		// Assert: all roots have expired, even root id should be removed
		std::initializer_list<NamespaceId::ValueType> ids{ 123u, 357u, 124u, 125u, 128u, 298u };
		AssertPrunedNamespaceIds(689, 0u, ids);
		AssertPrunedNamespaceIds(700, 0u, ids);
		AssertPrunedNamespaceIds(10000, 0u, ids);
	}

	TEST(TEST_CLASS, PruneCanBeCalledOnEmptyHistory) {
		// Arrange:
		RootNamespaceHistory history(NamespaceId(123));

		// Sanity:
		EXPECT_TRUE(history.empty());

		// Act:
		history.prune(Height(234));

		// Assert:
		EXPECT_TRUE(history.empty());
	}

	TEST(TEST_CLASS, PruneDoesNotRemoveExpiredRootsWithinGracePeriod) {
		for (auto i = 0u; i < Grace_Period_Duration; ++i) {
			// Arrange:
			RootNamespaceHistory history(NamespaceId(123));
			history.push_back(test::CreateRandomOwner(), test::CreateLifetime(234, 321, Grace_Period_Duration));

			// Sanity:
			EXPECT_EQ(1u, history.historyDepth());

			// Act + Assert:
			EXPECT_TRUE(history.prune(Height(321 + i)).empty()) << "for duration " << i;
		}
	}

	TEST(TEST_CLASS, PrunePrunesExpiredRootsAfterGracePeriodEnds) {
		for (auto offset : { 0u, 1u, 1000u, 1234567u }) {
			// Arrange:
			RootNamespaceHistory history(NamespaceId(123));
			history.push_back(test::CreateRandomOwner(), test::CreateLifetime(234, 321, Grace_Period_Duration));

			// Sanity:
			EXPECT_EQ(1u, history.historyDepth());

			// Act + Assert:
			auto expectedIds = std::set<NamespaceId>{ NamespaceId(123) };
			EXPECT_EQ(expectedIds, history.prune(Height(321 + Grace_Period_Duration + offset))) << "for offset " << offset;
		}
	}

	// endregion

	// region isActive

	TEST(TEST_CLASS, IsActiveReturnsFalseWhenHistoryIsEmpty) {
		// Arrange:
		RootNamespaceHistory history(NamespaceId(123));

		// Sanity:
		EXPECT_TRUE(history.empty());

		// Act + Assert:
		EXPECT_FALSE(history.isActive(Height(250)));
	}

	TEST(TEST_CLASS, IsActiveReturnsCorrectValueWhenHistoryIsNotEmpty) {
		// Arrange:
		// - root1 expires at height 173
		// - root2 expires at height 273
		auto owner = test::GenerateRandomByteArray<Address>();
		RootNamespaceHistory history(NamespaceId(123));
		history.push_back(owner, test::CreateLifetime(123, 173, 50));
		history.push_back(owner, test::CreateLifetime(173, 273, 50));

		// Act + Assert:
		EXPECT_FALSE(history.isActive(Height(50)));
		EXPECT_FALSE(history.isActive(Height(122)));
		EXPECT_FALSE(history.isActive(Height(123))); // isActive only checks latest lifetime
		EXPECT_FALSE(history.isActive(Height(172))); // isActive only checks latest lifetime

		EXPECT_TRUE(history.isActive(Height(173)));
		EXPECT_TRUE(history.isActive(Height(250)));
		EXPECT_TRUE(history.isActive(Height(272)));
		EXPECT_TRUE(history.isActive(Height(322)));

		EXPECT_FALSE(history.isActive(Height(323)));
		EXPECT_FALSE(history.isActive(Height(350)));
	}

	// endregion
}}

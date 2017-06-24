#include "src/state/MosaicHistory.h"
#include "src/state/MosaicEntry.h"
#include "tests/test/MosaicTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

	namespace {
		constexpr NamespaceId Default_Namespace_Id(234);
		constexpr MosaicId Default_Mosaic_Id(345);

		auto CreateDefaultHistory() {
			return MosaicHistory(Default_Namespace_Id, Default_Mosaic_Id);
		}

		auto CreateMosaicDefinition(const Key& owner, uint64_t duration) {
			return MosaicDefinition(Height(123), owner, test::CreateMosaicPropertiesWithDuration(ArtifactDuration(duration)));
		}

		auto CreateDefaultMosaicDefinition(const Key& owner) {
			return CreateMosaicDefinition(owner, Eternal_Artifact_Duration.unwrap());
		}

		auto CreateHistoryWithSupplies(const MosaicDefinition& definition, const std::vector<Amount::ValueType>& supplies) {
			auto history = CreateDefaultHistory();
			for (auto supply : supplies)
				history.push_back(definition, Amount(supply));

			// Sanity:
			EXPECT_EQ(Default_Namespace_Id, history.namespaceId());
			EXPECT_EQ(Default_Mosaic_Id, history.id());
			EXPECT_FALSE(history.empty());
			EXPECT_EQ(supplies.size(), history.historyDepth());

			return history;
		}

		void AssertMosaicDefinition(const MosaicDefinition& expectedDefinition, const MosaicDefinition& actualDefinition) {
			EXPECT_EQ(expectedDefinition.height(), actualDefinition.height());
			EXPECT_EQ(expectedDefinition.owner(), actualDefinition.owner());

			test::AssertMosaicDefinitionProperties(expectedDefinition.properties(), actualDefinition.properties());
		}

		void AssertMosaicEntry(const MosaicDefinition& expectedDefinition, Amount expectedSupply, const MosaicEntry& actualEntry) {
			AssertMosaicDefinition(expectedDefinition, actualEntry.definition());

			EXPECT_EQ(Default_Namespace_Id, actualEntry.namespaceId());
			EXPECT_EQ(Default_Mosaic_Id, actualEntry.mosaicId());
			EXPECT_EQ(expectedSupply, actualEntry.supply());
		}

		std::vector<Amount> GetSupplies(const MosaicHistory& history) {
			// check the supplies as a way to validate iteration
			std::vector<Amount> supplies;
			for (auto iter = history.cbegin(); history.cend() != iter; ++iter)
				supplies.push_back(iter->supply());

			return supplies;
		}
	}

	// region ctor

	TEST(MosaicHistoryTests, CanCreateEmptyMosaicHistory) {
		// Act:
		auto history = CreateDefaultHistory();

		// Assert:
		EXPECT_EQ(Default_Namespace_Id, history.namespaceId());
		EXPECT_EQ(Default_Mosaic_Id, history.id());
		EXPECT_TRUE(history.empty());
		EXPECT_EQ(0u, history.historyDepth());
		EXPECT_EQ(std::vector<Amount>(), GetSupplies(history));
	}

	// endregion

	// region push_back

	TEST(MosaicHistoryTests, CanAddSingleMosaicEntry) {
		// Arrange:
		auto owner = test::GenerateRandomData<Key_Size>();
		auto definition = CreateDefaultMosaicDefinition(owner);
		auto history = CreateDefaultHistory();

		// Act:
		history.push_back(definition, Amount(567));

		// Assert:
		EXPECT_EQ(Default_Namespace_Id, history.namespaceId());
		EXPECT_EQ(Default_Mosaic_Id, history.id());
		EXPECT_FALSE(history.empty());
		EXPECT_EQ(1u, history.historyDepth());
		EXPECT_EQ(std::vector<Amount> ({ Amount(567) }), GetSupplies(history));

		AssertMosaicEntry(definition, Amount(567), history.back());
	}

	TEST(MosaicHistoryTests, CanAddMultipleMosaicEntries) {
		// Arrange:
		auto owner = test::GenerateRandomData<Key_Size>();
		auto definition1 = CreateDefaultMosaicDefinition(owner);
		auto definition2 = CreateDefaultMosaicDefinition(owner);
		auto definition3 = CreateDefaultMosaicDefinition(owner);
		auto history = CreateDefaultHistory();

		// Act:
		history.push_back(definition1, Amount(567));
		history.push_back(definition2, Amount(678));
		history.push_back(definition3, Amount(789));

		// Assert:
		EXPECT_EQ(Default_Namespace_Id, history.namespaceId());
		EXPECT_EQ(Default_Mosaic_Id, history.id());
		EXPECT_FALSE(history.empty());
		EXPECT_EQ(3u, history.historyDepth());
		EXPECT_EQ(std::vector<Amount> ({ Amount(567), Amount(678), Amount(789) }), GetSupplies(history));

		AssertMosaicEntry(definition3, Amount(789), history.back());
	}

	// endregion

	// region pop_back

	TEST(MosaicHistoryTests, PopBackRemovesLastMosaicEntry) {
		// Arrange:
		auto owner = test::GenerateRandomData<Key_Size>();
		auto definition = CreateDefaultMosaicDefinition(owner);
		auto history = CreateHistoryWithSupplies(definition, { 567, 678 });

		// Act:
		history.pop_back();

		// Assert:
		EXPECT_EQ(Default_Namespace_Id, history.namespaceId());
		EXPECT_EQ(Default_Mosaic_Id, history.id());
		EXPECT_FALSE(history.empty());
		EXPECT_EQ(1u, history.historyDepth());
		EXPECT_EQ(std::vector<Amount> ({ Amount(567) }), GetSupplies(history));

		AssertMosaicEntry(definition, Amount(567), history.back());
	}

	// endregion

	// region back

	TEST(MosaicHistoryTests, BackReturnsMostRecentMosaicEntry) {
		// Arrange:
		auto owner = test::GenerateRandomData<Key_Size>();
		auto definition = CreateDefaultMosaicDefinition(owner);
		auto history = CreateHistoryWithSupplies(definition, { 567, 678, 789 });

		// Act:
		auto& entry = history.back();
		auto& constEntry = const_cast<const MosaicHistory&>(history).back();

		// Assert:
		EXPECT_EQ(Default_Namespace_Id, history.namespaceId());
		EXPECT_EQ(Default_Mosaic_Id, history.id());
		EXPECT_FALSE(history.empty());
		EXPECT_EQ(3u, history.historyDepth());
		EXPECT_EQ(std::vector<Amount> ({ Amount(567), Amount(678), Amount(789) }), GetSupplies(history));
		EXPECT_EQ(&constEntry, &entry);

		AssertMosaicEntry(definition, Amount(789), entry);
		AssertMosaicEntry(definition, Amount(789), constEntry);
	}

	// endregion

	// region prune

	TEST(MosaicHistoryTests, PruneDoesNotRemoveEternalMosaicEntries) {
		// Arrange: default mosaic definition has eternal lifetime
		auto owner = test::GenerateRandomData<Key_Size>();
		auto definition = CreateDefaultMosaicDefinition(owner);
		auto history = CreateHistoryWithSupplies(definition, { 567, 678, 789 });

		// Act:
		history.prune(Height(321));
		history.prune(Height(10000));
		history.prune(Height(10000000));

		// Assert:
		EXPECT_EQ(Default_Namespace_Id, history.namespaceId());
		EXPECT_EQ(Default_Mosaic_Id, history.id());
		EXPECT_FALSE(history.empty());
		EXPECT_EQ(3u, history.historyDepth());
		EXPECT_EQ(std::vector<Amount> ({ Amount(567), Amount(678), Amount(789) }), GetSupplies(history));
	}

	TEST(MosaicHistoryTests, PruneRemovesMostRecentExpiredMosaicEntryAndAllPriorMosaicEntries_OrderedExpiry) {
		// Arrange: mosaic definition has height 123
		// - create three definitions:
		// - definition1 expires at height 123 + 50 = 173
		// - definition2 expires at height 123 + 150 = 273
		// - definition3 expires at height 123 + 250 = 373
		auto owner = test::GenerateRandomData<Key_Size>();
		auto definition1 = CreateMosaicDefinition(owner, 50);
		auto definition2 = CreateMosaicDefinition(owner, 150);
		auto definition3 = CreateMosaicDefinition(owner, 250);
		auto history = CreateHistoryWithSupplies(definition1, { 123 });
		history.push_back(definition2, Amount(234));
		history.push_back(definition3, Amount(345));

		// Act: first two entries should be pruned
		history.prune(Height(300));

		// Assert:
		EXPECT_EQ(Default_Namespace_Id, history.namespaceId());
		EXPECT_EQ(Default_Mosaic_Id, history.id());
		EXPECT_FALSE(history.empty());
		EXPECT_EQ(1u, history.historyDepth());
		EXPECT_EQ(std::vector<Amount> ({ Amount(345) }), GetSupplies(history));

		AssertMosaicEntry(definition3, Amount(345), history.back());
	}

	TEST(MosaicHistoryTests, PruneRemovesMostRecentExpiredMosaicEntryAndAllPriorMosaicEntries_UnorderedExpiry) {
		// Arrange: mosaic definition has height 123
		// - create four definitions:
		// - definition1 expires at height 123 + 50 = 173
		// - definition2 expires at height 123 + 250 = 373
		// - definition3 expires at height 123 + 150 = 273
		// - definition4 expires at height 123 + 250 = 373
		auto owner = test::GenerateRandomData<Key_Size>();
		auto definition1 = CreateMosaicDefinition(owner, 50);
		auto definition2 = CreateMosaicDefinition(owner, 250);
		auto definition3 = CreateMosaicDefinition(owner, 150);
		auto definition4 = CreateMosaicDefinition(owner, 250);
		auto history = CreateHistoryWithSupplies(definition1, { 123 });
		history.push_back(definition2, Amount(234));
		history.push_back(definition3, Amount(345));
		history.push_back(definition4, Amount(456));

		// Act: first three entries should be pruned
		history.prune(Height(300));

		// Assert:
		EXPECT_EQ(Default_Namespace_Id, history.namespaceId());
		EXPECT_EQ(Default_Mosaic_Id, history.id());
		EXPECT_FALSE(history.empty());
		EXPECT_EQ(1u, history.historyDepth());
		EXPECT_EQ(std::vector<Amount> ({ Amount(456) }), GetSupplies(history));

		AssertMosaicEntry(definition4, Amount(456), history.back());
	}

	TEST(MosaicHistoryTests, PruneCanRemoveAllEntries) {
		// Arrange: mosaic definition has height 123
		// - create three definitions:
		// - definition1 expires at height 123 + 50 = 173
		// - definition2 expires at height 123 + 150 = 273
		// - definition3 expires at height 123 + 250 = 373
		auto owner = test::GenerateRandomData<Key_Size>();
		auto definition1 = CreateMosaicDefinition(owner, 50);
		auto definition2 = CreateMosaicDefinition(owner, 150);
		auto definition3 = CreateMosaicDefinition(owner, 250);
		auto history = CreateHistoryWithSupplies(definition1, { 123 });
		history.push_back(definition2, Amount(234));
		history.push_back(definition3, Amount(345));

		// Act: all entries should be pruned
		history.prune(Height(400));

		// Assert:
		EXPECT_EQ(Default_Namespace_Id, history.namespaceId());
		EXPECT_EQ(Default_Mosaic_Id, history.id());
		EXPECT_TRUE(history.empty());
		EXPECT_EQ(0u, history.historyDepth());
		EXPECT_EQ(std::vector<Amount>(), GetSupplies(history));
	}

	TEST(MosaicHistoryTests, PruneCanBeCalledOnEmptyHistory) {
		// Arrange:
		auto history = CreateDefaultHistory();

		// Sanity:
		EXPECT_TRUE(history.empty());

		// Act:
		history.prune(Height(234));

		// Assert:
		EXPECT_TRUE(history.empty());
	}

	// endregion
}}

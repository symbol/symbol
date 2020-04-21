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

#include "harvesting/src/UnlockedAccountsUpdater.h"
#include "harvesting/src/UnlockedAccounts.h"
#include "harvesting/src/UnlockedFileQueueConsumer.h"
#include "catapult/io/FileQueue.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "harvesting/tests/test/UnlockedTestEntry.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace harvesting {

#define TEST_CLASS UnlockedAccountsUpdaterTests

	namespace {
		constexpr auto Harvesting_Mosaic_Id = MosaicId(9876);

		// region test context

		class TestContext {
		public:
			TestContext()
					: m_dataDirectory(config::CatapultDataDirectory(m_directoryGuard.name()))
					, m_config(CreateBlockChainConfiguration())
					, m_cache(test::CreateEmptyCatapultCache(m_config))
					, m_unlockedAccounts(10, [](const auto&) { return 0; })
					, m_keyPair(test::GenerateKeyPair())
					, m_updater(m_cache, m_unlockedAccounts, m_keyPair, m_dataDirectory) {
				auto descriptor = BlockGeneratorAccountDescriptor(test::GenerateKeyPair(), test::GenerateKeyPair());
				auto signingKeyPair = descriptor.signingKeyPair().publicKey();
				auto vrfPublicKey = descriptor.vrfKeyPair().publicKey();
				m_unlockedAccounts.modifier().add(std::move(descriptor));
				addAccount(signingKeyPair, vrfPublicKey, Amount(1234));
			}

		public:
			size_t numUnlockedAccounts() const {
				return m_unlockedAccounts.view().size();
			}

			auto harvestersFilename() const {
				return m_dataDirectory.rootDir().file("harvesters.dat");
			}

			void load() {
				m_updater.load();
			}

			void update() {
				m_updater.update();
			}

		public:
			auto queueAddMessageWithHarvester(const crypto::KeyPair& ephemeralKeyPair, const BlockGeneratorAccountDescriptor& descriptor) {
				io::FileQueueWriter writer(m_dataDirectory.dir("transfer_message").str());
				auto testEntry = test::PrepareUnlockedTestEntry(
						ephemeralKeyPair,
						m_keyPair.publicKey(),
						test::ToClearTextBuffer(descriptor));

				io::Write8(writer, utils::to_underlying_type(UnlockedEntryDirection::Add));
				writer.write({ reinterpret_cast<const uint8_t*>(&testEntry), sizeof(testEntry) });
				writer.flush();
				return testEntry;
			}

			auto queueAddMessageWithHarvester(const BlockGeneratorAccountDescriptor& descriptor) {
				return queueAddMessageWithHarvester(test::GenerateKeyPair(), descriptor);
			}

			void queueRemoveMessage(const test::UnlockedTestEntry& testEntry) {
				io::FileQueueWriter writer(m_dataDirectory.dir("transfer_message").str());
				io::Write8(writer, utils::to_underlying_type(UnlockedEntryDirection::Remove));
				writer.write({ reinterpret_cast<const uint8_t*>(&testEntry), sizeof(testEntry) });
				writer.flush();
			}

			void appendHarvesterDirectlyToHarvestersFile(const BlockGeneratorAccountDescriptor& descriptor) {
				io::RawFile file(harvestersFilename(), io::OpenMode::Read_Append);
				file.seek(file.size());

				auto testEntry = test::PrepareUnlockedTestEntry(m_keyPair.publicKey(), test::ToClearTextBuffer(descriptor));
				file.write({ reinterpret_cast<const uint8_t*>(&testEntry), sizeof(testEntry) });
			}

			void addEnabledAccount(const BlockGeneratorAccountDescriptor& descriptor) {
				addAccount(descriptor.signingKeyPair().publicKey(), test::GenerateRandomByteArray<Key>(), Amount(1111));
			}

			void assertHarvesterFileEntries(const test::UnlockedTestEntries& expectedEntries) {
				test::AssertUnlockedEntriesFileContents(harvestersFilename(), expectedEntries);
			}

			void assertNoHarvesterFile() {
				EXPECT_FALSE(boost::filesystem::exists(harvestersFilename()));
			}

		private:
			void addAccount(const Key& signingPublicKey, const Key& vrfPublicKey, Amount balance = Amount(0)) {
				auto delta = m_cache.createDelta();
				auto& accountStateCache = delta.sub<cache::AccountStateCache>();
				accountStateCache.addAccount(signingPublicKey, Height(100));

				auto accountStateIter = accountStateCache.find(signingPublicKey);
				auto& accountState = accountStateIter.get();
				accountState.Balances.credit(Harvesting_Mosaic_Id, balance);
				accountState.ImportanceSnapshots.set(Importance(1000), model::ImportanceHeight(100));
				accountState.SupplementalAccountKeys.set(state::AccountKeyType::VRF, vrfPublicKey);
				m_cache.commit(Height(100));
			}

			static model::BlockChainConfiguration CreateBlockChainConfiguration() {
				auto config = model::BlockChainConfiguration::Uninitialized();
				config.Network.Identifier = model::NetworkIdentifier::Mijin_Test;
				config.EnableVerifiableState = false;
				config.EnableVerifiableReceipts = false;
				config.HarvestingMosaicId = Harvesting_Mosaic_Id;
				config.ImportanceGrouping = 100;
				config.MinHarvesterBalance = Amount(1000);
				config.MaxHarvesterBalance = Amount(std::numeric_limits<Amount::ValueType>::max());
				return config;
			}

		private:
			test::TempDirectoryGuard m_directoryGuard;
			config::CatapultDataDirectory m_dataDirectory;
			model::BlockChainConfiguration m_config;
			cache::CatapultCache m_cache;
			UnlockedAccounts m_unlockedAccounts;
			crypto::KeyPair m_keyPair;
			UnlockedAccountsUpdater m_updater;
		};

		// endregion
	}

	// region load

	TEST(TEST_CLASS, LoadAddsToUnlockedAccounts) {
		// Arrange:
		TestContext context;
		for (auto i = 0; i < 4; ++i)
			context.appendHarvesterDirectlyToHarvestersFile(test::GenerateRandomAccountDescriptors(1)[0]);

		// Sanity:
		EXPECT_EQ(1u, context.numUnlockedAccounts());

		// Act:
		context.load();

		// Assert:
		EXPECT_EQ(5u, context.numUnlockedAccounts());
	}

	// endregion

	// region update - pruning + limits

	TEST(TEST_CLASS, UpdateDoesNotPruneValidHarvester) {
		// Arrange: test context always contains single unlocked account
		TestContext context;

		// Sanity:
		EXPECT_EQ(1u, context.numUnlockedAccounts());

		// Act:
		context.update();

		// Assert:
		EXPECT_EQ(1u, context.numUnlockedAccounts());
	}

	TEST(TEST_CLASS, UpdateRemovesFileWhenAllAccountsArePruned) {
		// Arrange: add non eligible account to queue
		TestContext context;
		auto descriptors = test::GenerateRandomAccountDescriptors(1);
		context.queueAddMessageWithHarvester(descriptors[0]);

		// Sanity:
		EXPECT_EQ(1u, context.numUnlockedAccounts());

		// Act:
		context.update();

		// Assert: entry was consumed, added to unlocked, pruned, so should not be saved to file
		EXPECT_EQ(1u, context.numUnlockedAccounts());
		context.assertNoHarvesterFile();
	}

	TEST(TEST_CLASS, UpdateSavesOnlyNonPrunedAccounts) {
		// Arrange:
		TestContext context;
		auto descriptors = test::GenerateRandomAccountDescriptors(3);
		auto entry1 = context.queueAddMessageWithHarvester(descriptors[0]);
		context.queueAddMessageWithHarvester(descriptors[1]);
		auto entry3 = context.queueAddMessageWithHarvester(descriptors[2]);

		// - add accounts 1 and 3 to cache
		context.addEnabledAccount(descriptors[0]);
		context.addEnabledAccount(descriptors[2]);

		// Sanity:
		EXPECT_EQ(1u, context.numUnlockedAccounts());

		// Act:
		context.update();

		// Assert: only eligible entries were added to unlocked accounts and to unlocked harvesters file
		EXPECT_EQ(3u, context.numUnlockedAccounts());
		context.assertHarvesterFileEntries({ entry1, entry3 });
	}

	TEST(TEST_CLASS, UpdateDoesNotSaveAccountWhenMaxUnlockedHasBeenReached) {
		// Arrange:
		TestContext context;

		// Sanity:
		EXPECT_EQ(1u, context.numUnlockedAccounts());

		// Act:
		test::UnlockedTestEntries expectedEntries;
		for (auto i = 0u; i < 15; ++i) {
			auto descriptors = test::GenerateRandomAccountDescriptors(1);
			auto entry = context.queueAddMessageWithHarvester(descriptors[0]);
			context.addEnabledAccount(descriptors[0]);

			// - max unlocked accounts is 10 and initially there was one account, so only first 9 will be added
			if (i < 9)
				expectedEntries.insert(entry);

			context.update();
		}

		// Assert:
		EXPECT_EQ(10u, context.numUnlockedAccounts());
		context.assertHarvesterFileEntries(expectedEntries);
	}

	// endregion

	// region update - save + remove (single account)

	TEST(TEST_CLASS, UpdateSavesAddedAccount) {
		// Arrange: add account to cache
		TestContext context;
		auto descriptors = test::GenerateRandomAccountDescriptors(1);
		context.addEnabledAccount(descriptors[0]);

		// - prepare entry
		auto entry = context.queueAddMessageWithHarvester(descriptors[0]);

		// Sanity:
		EXPECT_EQ(1u, context.numUnlockedAccounts());

		// Act:
		context.update();

		// Assert: entry was consumed, added to unlocked accounts and added to unlocked harvesters file
		EXPECT_EQ(2u, context.numUnlockedAccounts());
		context.assertHarvesterFileEntries({ entry });
	}

	TEST(TEST_CLASS, UpdateDoesNotSaveRemovedAccount) {
		// Arrange: add account to cache
		TestContext context;
		auto descriptors = test::GenerateRandomAccountDescriptors(1);
		context.addEnabledAccount(descriptors[0]);

		// - prepare and process entry
		auto entry = context.queueAddMessageWithHarvester(descriptors[0]);
		context.update();

		// Sanity:
		EXPECT_EQ(2u, context.numUnlockedAccounts());

		// - prepare removal entry
		context.queueRemoveMessage(entry);

		// Act:
		context.update();

		// Assert: entry was consumed, removed from unlocked accounts and removed unlocked harvesters file
		EXPECT_EQ(1u, context.numUnlockedAccounts());
		context.assertNoHarvesterFile();
	}

	// endregion

	// region update - save + remove (duplicate harvester)

	TEST(TEST_CLASS, UpdateDeduplicatesAddedHarvesters) {
		// Arrange: add accounts to cache
		TestContext context;
		auto descriptors = test::GenerateRandomAccountDescriptors(2);
		context.addEnabledAccount(descriptors[0]);
		context.addEnabledAccount(descriptors[1]);

		// - prepare and process non-consecutive entries with same harvester
		auto entry1 = context.queueAddMessageWithHarvester(descriptors[0]);
		auto entry2 = context.queueAddMessageWithHarvester(descriptors[1]);
		context.queueAddMessageWithHarvester(descriptors[0]);

		// Sanity:
		EXPECT_EQ(1u, context.numUnlockedAccounts());

		// Act:
		context.update();

		// Assert: only one entry was added per harvester
		EXPECT_EQ(3u, context.numUnlockedAccounts());
		context.assertHarvesterFileEntries({ entry1, entry2 });
	}

	TEST(TEST_CLASS, UpdateAllowsRemovalOfSameHarvesterMultipleTimes) {
		// Arrange: add accounts to cache
		TestContext context;
		auto descriptors = test::GenerateRandomAccountDescriptors(2);
		context.addEnabledAccount(descriptors[0]);
		context.addEnabledAccount(descriptors[1]);

		// - prepare and process non-consecutive entries with same harvester
		auto entry1 = context.queueAddMessageWithHarvester(descriptors[0]);
		auto entry2 = context.queueAddMessageWithHarvester(descriptors[1]);
		auto entry3 = context.queueAddMessageWithHarvester(descriptors[0]);
		context.update();

		// Sanity:
		EXPECT_EQ(3u, context.numUnlockedAccounts());

		// Act: remove the third entry
		context.queueRemoveMessage(entry3);
		context.update();

		// Assert:
		// - removes the entry from unlocked accounts (because harvester matches)
		// - removes the entry from the storage (because announcer matches)
		EXPECT_EQ(2u, context.numUnlockedAccounts());
		context.assertHarvesterFileEntries({ entry2 });

		// Act: remove the second entry
		context.queueRemoveMessage(entry2);
		context.update();

		// Assert:
		EXPECT_EQ(1u, context.numUnlockedAccounts());
		context.assertNoHarvesterFile();

		// Act: remove the first entry
		context.queueRemoveMessage(entry1);
		context.update();

		// Assert: no change (remove is skipped because harvester is already locked)
		EXPECT_EQ(1u, context.numUnlockedAccounts());
		context.assertNoHarvesterFile();
	}

	// endregion

	// region update - save + remove (duplicate announcer)

	TEST(TEST_CLASS, UpdateDeduplicatesAddedAnnouncers) {
		// Arrange: add accounts to cache
		TestContext context;
		auto descriptors = test::GenerateRandomAccountDescriptors(3);
		context.addEnabledAccount(descriptors[0]);
		context.addEnabledAccount(descriptors[1]);
		context.addEnabledAccount(descriptors[2]);

		// - prepare and process non-consecutive entries with same announcer
		auto ephemeralKeyPair = test::GenerateKeyPair();
		auto entry1 = context.queueAddMessageWithHarvester(ephemeralKeyPair, descriptors[0]);
		auto entry2 = context.queueAddMessageWithHarvester(descriptors[1]);
		context.queueAddMessageWithHarvester(ephemeralKeyPair, descriptors[2]);

		// Sanity:
		EXPECT_EQ(1u, context.numUnlockedAccounts());

		// Act:
		context.update();

		// Assert: only one entry was added per announcer
		EXPECT_EQ(3u, context.numUnlockedAccounts());
		context.assertHarvesterFileEntries({ entry1, entry2 });
	}

	TEST(TEST_CLASS, UpdateAllowsRemovalOfSameAnnouncerMultipleTimes) {
		// Arrange: add accounts to cache
		TestContext context;
		auto descriptors = test::GenerateRandomAccountDescriptors(3);
		context.addEnabledAccount(descriptors[0]);
		context.addEnabledAccount(descriptors[1]);
		context.addEnabledAccount(descriptors[2]);

		// - prepare and process non-consecutive entries with same announcer
		auto ephemeralKeyPair = test::GenerateKeyPair();
		auto entry1 = context.queueAddMessageWithHarvester(ephemeralKeyPair, descriptors[0]);
		auto entry2 = context.queueAddMessageWithHarvester(descriptors[1]);
		auto entry3 = context.queueAddMessageWithHarvester(ephemeralKeyPair, descriptors[2]);
		context.update();

		// Sanity:
		EXPECT_EQ(3u, context.numUnlockedAccounts());

		// Act: remove the third entry
		context.queueRemoveMessage(entry3);
		context.update();

		// Assert:
		// - removes the entry from the storage (because announcer matches)
		EXPECT_EQ(3u, context.numUnlockedAccounts());
		context.assertHarvesterFileEntries({ entry2 });

		// Act: remove the second entry
		context.queueRemoveMessage(entry2);
		context.update();

		// Assert:
		EXPECT_EQ(2u, context.numUnlockedAccounts());
		context.assertNoHarvesterFile();

		// Act: remove the first entry
		context.queueRemoveMessage(entry1);
		context.update();

		// Assert:
		// - removes the entry from unlocked accounts (because harvester matches)
		EXPECT_EQ(1u, context.numUnlockedAccounts());
		context.assertNoHarvesterFile();
	}

	// endregion

	// region update - save + remove (duplicate harvester+announcer pair)

	TEST(TEST_CLASS, UpdateDeduplicatesAddedAnnouncerAndHarvesterPairs) {
		// Arrange: add accounts to cache
		TestContext context;
		auto descriptors = test::GenerateRandomAccountDescriptors(2);
		context.addEnabledAccount(descriptors[0]);
		context.addEnabledAccount(descriptors[1]);

		// - prepare and process non-consecutive entries with same announcer and harvester
		auto ephemeralKeyPair = test::GenerateKeyPair();
		auto entry1 = context.queueAddMessageWithHarvester(ephemeralKeyPair, descriptors[0]);
		auto entry2 = context.queueAddMessageWithHarvester(descriptors[1]);
		context.queueAddMessageWithHarvester(ephemeralKeyPair, descriptors[0]);

		// Sanity:
		EXPECT_EQ(1u, context.numUnlockedAccounts());

		// Act:
		context.update();

		// Assert: only one entry was added per announcer
		EXPECT_EQ(3u, context.numUnlockedAccounts());
		context.assertHarvesterFileEntries({ entry1, entry2 });
	}

	TEST(TEST_CLASS, UpdateAllowsRemovalOfSameAnnouncerAndHarvesterPairMultipleTimes) {
		// Arrange: add accounts to cache
		TestContext context;
		auto descriptors = test::GenerateRandomAccountDescriptors(2);
		context.addEnabledAccount(descriptors[0]);
		context.addEnabledAccount(descriptors[1]);

		// - prepare and process non-consecutive entries with same announcer and harvester
		auto ephemeralKeyPair = test::GenerateKeyPair();
		auto entry1 = context.queueAddMessageWithHarvester(ephemeralKeyPair, descriptors[0]);
		auto entry2 = context.queueAddMessageWithHarvester(descriptors[1]);
		auto entry3 = context.queueAddMessageWithHarvester(ephemeralKeyPair, descriptors[0]);
		context.update();

		// Sanity:
		EXPECT_EQ(3u, context.numUnlockedAccounts());

		// Act: remove the third entry
		context.queueRemoveMessage(entry3);
		context.update();

		// Assert:
		// - removes the entry from unlocked accounts (because harvester matches)
		// - removes the entry from the storage (because announcer matches)
		EXPECT_EQ(2u, context.numUnlockedAccounts());
		context.assertHarvesterFileEntries({ entry2 });

		// Act: remove the second entry
		context.queueRemoveMessage(entry2);
		context.update();

		// Assert:
		EXPECT_EQ(1u, context.numUnlockedAccounts());
		context.assertNoHarvesterFile();

		// Act: remove the first entry
		context.queueRemoveMessage(entry1);
		context.update();

		// Assert: no change (remove is skipped because harvester is already locked)
		EXPECT_EQ(1u, context.numUnlockedAccounts());
		context.assertNoHarvesterFile();
	}

	// endregion
}}

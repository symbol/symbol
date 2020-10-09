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
#include "catapult/model/Address.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "harvesting/tests/test/HarvestRequestEncryptedPayload.h"
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
			enum class SeedOptions { Address, Public_Key };

		public:
			explicit TestContext(SeedOptions seedOptions = SeedOptions::Public_Key)
					: m_dataDirectory(config::CatapultDataDirectory(m_directoryGuard.name()))
					, m_config(CreateBlockChainConfiguration())
					, m_cache(test::CreateEmptyCatapultCache(m_config))
					, m_unlockedAccounts(10, [](const auto&) { return 0; })
					, m_keyPair(test::GenerateKeyPair())
					, m_updater(m_cache, m_unlockedAccounts, m_keyPair, m_dataDirectory) {
				auto descriptor = BlockGeneratorAccountDescriptor(test::GenerateKeyPair(), test::GenerateKeyPair());
				auto signingPublicKey = descriptor.signingKeyPair().publicKey();
				auto vrfPublicKey = descriptor.vrfKeyPair().publicKey();
				m_unlockedAccounts.modifier().add(std::move(descriptor));

				if (SeedOptions::Public_Key == seedOptions)
					addMainAccount(signingPublicKey, vrfPublicKey, Amount(1234));
				else
					addMainAccount(model::PublicKeyToAddress(signingPublicKey, m_config.Network.Identifier), vrfPublicKey, Amount(1234));
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
			void modifyAccount(const Key& publicKey, const consumer<state::AccountState&>& modify) {
				auto delta = m_cache.createDelta();
				auto& accountStateCache = delta.sub<cache::AccountStateCache>();

				modify(accountStateCache.find(publicKey).get());

				m_cache.commit(Height(100));
			}

			auto queueAddMessageWithHarvester(
					const crypto::KeyPair& ephemeralKeyPair,
					const BlockGeneratorAccountDescriptor& descriptor,
					const Key& mainAccountPublicKey) {
				auto encryptedPayload = test::PrepareHarvestRequestEncryptedPayload(
						ephemeralKeyPair,
						m_keyPair.publicKey(),
						test::ToClearTextBuffer(descriptor));

				io::FileQueueWriter writer(m_dataDirectory.dir("transfer_message").str());
				io::Write8(writer, utils::to_underlying_type(HarvestRequestOperation::Add));
				writer.write(mainAccountPublicKey);
				writer.write({ reinterpret_cast<const uint8_t*>(&encryptedPayload), sizeof(encryptedPayload) });
				writer.flush();

				return encryptedPayload;
			}

			auto queueAddMessageWithHarvester(const BlockGeneratorAccountDescriptor& descriptor, const Key& mainAccountPublicKey) {
				return queueAddMessageWithHarvester(test::GenerateKeyPair(), descriptor, mainAccountPublicKey);
			}

			auto queueAddMessageWithHarvester(const BlockGeneratorAccountDescriptor& descriptor) {
				return queueAddMessageWithHarvester(descriptor, test::GenerateRandomByteArray<Key>());
			}

			void queueRemoveMessage(const test::HarvestRequestEncryptedPayload& encryptedPayload, const Key& mainAccountPublicKey) {
				io::FileQueueWriter writer(m_dataDirectory.dir("transfer_message").str());
				io::Write8(writer, utils::to_underlying_type(HarvestRequestOperation::Remove));
				writer.write(mainAccountPublicKey);
				writer.write({ reinterpret_cast<const uint8_t*>(&encryptedPayload), sizeof(encryptedPayload) });
				writer.flush();
			}

			void appendHarvesterDirectlyToHarvestersFile(const BlockGeneratorAccountDescriptor& descriptor) {
				io::RawFile file(harvestersFilename(), io::OpenMode::Read_Append);
				file.seek(file.size());

				auto encryptedPayload = test::PrepareHarvestRequestEncryptedPayload(
						m_keyPair.publicKey(),
						test::ToClearTextBuffer(descriptor));
				file.write({ reinterpret_cast<const uint8_t*>(&encryptedPayload), sizeof(encryptedPayload) });
			}

			Key addEnabledAccount(const BlockGeneratorAccountDescriptor& descriptor) {
				return addAccount(descriptor.signingKeyPair().publicKey(), descriptor.vrfKeyPair().publicKey(), Amount(1111));
			}

			void assertHarvesterFileRecords(const test::HarvestRequestEncryptedPayloads& expectedEncryptedPayloads) {
				test::AssertHarvesterFileContents(harvestersFilename(), expectedEncryptedPayloads);
			}

			void assertNoHarvesterFile() {
				EXPECT_FALSE(boost::filesystem::exists(harvestersFilename()));
			}

		private:
			template<typename TAccountIdentifier>
			void addMainAccount(const TAccountIdentifier& accountIdentifier, const Key& vrfPublicKey, Amount balance) {
				auto delta = m_cache.createDelta();
				auto& accountStateCache = delta.sub<cache::AccountStateCache>();

				accountStateCache.addAccount(accountIdentifier, Height(100));
				auto mainAccountStateIter = accountStateCache.find(accountIdentifier);
				mainAccountStateIter.get().Balances.credit(Harvesting_Mosaic_Id, balance);
				mainAccountStateIter.get().ImportanceSnapshots.set(Importance(1000), model::ImportanceHeight(100));
				mainAccountStateIter.get().SupplementalPublicKeys.node().set(m_keyPair.publicKey());
				mainAccountStateIter.get().SupplementalPublicKeys.vrf().set(vrfPublicKey);

				m_cache.commit(Height(100));
			}

			Key addAccount(const Key& signingPublicKey, const Key& vrfPublicKey, Amount balance = Amount(0)) {
				auto mainAccountPublicKey = test::GenerateRandomByteArray<Key>();
				addMainAccount(mainAccountPublicKey, vrfPublicKey, balance);

				auto delta = m_cache.createDelta();
				auto& accountStateCache = delta.sub<cache::AccountStateCache>();
				auto mainAccountStateIter = accountStateCache.find(mainAccountPublicKey);
				mainAccountStateIter.get().AccountType = state::AccountType::Main;
				mainAccountStateIter.get().SupplementalPublicKeys.linked().set(signingPublicKey);

				accountStateCache.addAccount(signingPublicKey, Height(100));
				auto remoteAccountStateIter = accountStateCache.find(signingPublicKey);
				remoteAccountStateIter.get().AccountType = state::AccountType::Remote;
				remoteAccountStateIter.get().SupplementalPublicKeys.linked().set(mainAccountPublicKey);

				m_cache.commit(Height(100));

				return mainAccountPublicKey;
			}

		private:
			static model::BlockChainConfiguration CreateBlockChainConfiguration() {
				auto config = model::BlockChainConfiguration::Uninitialized();
				config.Network.Identifier = model::NetworkIdentifier::Private_Test;
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

		// region test utils

		void SetRandom(state::AccountPublicKeys::PublicKeyAccessor<Key>& publicKeyAccessor) {
			publicKeyAccessor.unset();
			publicKeyAccessor.set(test::GenerateRandomByteArray<Key>());
		}

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

	// region update - save single account

	TEST(TEST_CLASS, UpdateSavesValidAccount) {
		// Arrange:
		TestContext context;
		auto descriptors = test::GenerateRandomAccountDescriptors(1);
		auto mainAccountPublicKey = context.addEnabledAccount(descriptors[0]);

		auto encryptedPayload = context.queueAddMessageWithHarvester(descriptors[0], mainAccountPublicKey);

		// Sanity:
		EXPECT_EQ(1u, context.numUnlockedAccounts());

		// Act:
		context.update();

		// Assert: valid account was added to unlocked accounts and to unlocked harvesters file
		EXPECT_EQ(2u, context.numUnlockedAccounts());
		context.assertHarvesterFileRecords({ encryptedPayload });
	}

	TEST(TEST_CLASS, UpdateBypassesInvalidAccount_UnknownMainAccount) {
		// Arrange:
		TestContext context;
		auto descriptors = test::GenerateRandomAccountDescriptors(1);
		context.addEnabledAccount(descriptors[0]);

		context.queueAddMessageWithHarvester(descriptors[0]);

		// Sanity:
		EXPECT_EQ(1u, context.numUnlockedAccounts());

		// Act:
		context.update();

		// Assert: ineligible message was ignored
		EXPECT_EQ(1u, context.numUnlockedAccounts());
		context.assertNoHarvesterFile();
	}

	namespace {
		void AssertUpdateBypasses(const consumer<state::AccountState&>& corruptAccountState) {
			// Arrange:
			TestContext context;
			auto descriptors = test::GenerateRandomAccountDescriptors(1);
			auto mainAccountPublicKey = context.addEnabledAccount(descriptors[0]);

			// - corrupt the account
			context.modifyAccount(mainAccountPublicKey, corruptAccountState);

			context.queueAddMessageWithHarvester(descriptors[0], mainAccountPublicKey);

			// Sanity:
			EXPECT_EQ(1u, context.numUnlockedAccounts());

			// Act:
			context.update();

			// Assert: ineligible message was ignored
			EXPECT_EQ(1u, context.numUnlockedAccounts());
			context.assertNoHarvesterFile();
		}
	}

	TEST(TEST_CLASS, UpdateBypassesInvalidAccount_MismatchedLinkedPublicKey) {
		AssertUpdateBypasses([](auto& accountState) {
			SetRandom(accountState.SupplementalPublicKeys.linked());
		});
	}

	TEST(TEST_CLASS, UpdateBypassesInvalidAccount_MismatchedNodePublicKey) {
		AssertUpdateBypasses([](auto& accountState) {
			SetRandom(accountState.SupplementalPublicKeys.node());
		});
	}

	TEST(TEST_CLASS, UpdateBypassesInvalidAccount_MismatchedVrfPublicKey) {
		AssertUpdateBypasses([](auto& accountState) {
			SetRandom(accountState.SupplementalPublicKeys.vrf());
		});
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

	TEST(TEST_CLASS, UpdateDoesNotPruneValidHarvesterWhenSigningPublicKeyIsNotInCache) {
		// Arrange:
		TestContext context(TestContext::SeedOptions::Address);

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

		// Assert: message was consumed, added to unlocked, pruned, so should not be saved to file
		EXPECT_EQ(1u, context.numUnlockedAccounts());
		context.assertNoHarvesterFile();
	}

	namespace {
		void AssertPruned(const consumer<state::AccountState&>& corruptAccountState) {
			// Arrange: add three accounts to cache
			TestContext context;
			auto descriptors = test::GenerateRandomAccountDescriptors(3);
			auto mainAccountPublicKey1 = context.addEnabledAccount(descriptors[0]);
			auto mainAccountPublicKey2 = context.addEnabledAccount(descriptors[1]);
			auto mainAccountPublicKey3 = context.addEnabledAccount(descriptors[2]);

			auto encryptedPayload1 = context.queueAddMessageWithHarvester(descriptors[0], mainAccountPublicKey1);
			context.queueAddMessageWithHarvester(descriptors[1], mainAccountPublicKey2);
			auto encryptedPayload3 = context.queueAddMessageWithHarvester(descriptors[2], mainAccountPublicKey3);

			// - trigger initial save
			context.update();

			// Sanity:
			EXPECT_EQ(4u, context.numUnlockedAccounts());

			// Act: corrupt the second account and resave
			context.modifyAccount(mainAccountPublicKey2, corruptAccountState);
			context.update();

			// Assert: only eligible messages remain in unlocked accounts and unlocked harvesters file
			EXPECT_EQ(3u, context.numUnlockedAccounts());
			context.assertHarvesterFileRecords({ encryptedPayload1, encryptedPayload3 });
		}
	}

	TEST(TEST_CLASS, UpdateSavePrunesMismatchedLinkedPublicKeyAccounts) {
		// Assert: linkedPublicKey mismatch is fatal error raised by RequireLinkedRemoteAndMainAccounts indicating state corruption
		EXPECT_THROW(AssertPruned([](auto& accountState) {
			SetRandom(accountState.SupplementalPublicKeys.linked());
		}), catapult_runtime_error);
	}

	TEST(TEST_CLASS, UpdateSavePrunesMismatchedNodePublicKeyAccounts) {
		AssertPruned([](auto& accountState) {
			SetRandom(accountState.SupplementalPublicKeys.node());
		});
	}

	TEST(TEST_CLASS, UpdateSavePrunesMismatchedVrfPublicKeyAccounts) {
		AssertPruned([](auto& accountState) {
			SetRandom(accountState.SupplementalPublicKeys.vrf());
		});
	}

	TEST(TEST_CLASS, UpdateSavePrunesIneligbileHarvesterAccounts) {
		AssertPruned([](auto& accountState) {
			accountState.Balances.debit(Harvesting_Mosaic_Id, accountState.Balances.get(Harvesting_Mosaic_Id));
		});
	}

	TEST(TEST_CLASS, UpdateDoesNotSaveAccountWhenMaxUnlockedHasBeenReached) {
		// Arrange:
		TestContext context;

		// Sanity:
		EXPECT_EQ(1u, context.numUnlockedAccounts());

		// Act:
		test::HarvestRequestEncryptedPayloads expectedEncryptedPayloads;
		for (auto i = 0u; i < 15; ++i) {
			auto descriptors = test::GenerateRandomAccountDescriptors(1);
			auto mainAccountPublicKey = context.addEnabledAccount(descriptors[0]);
			auto encryptedPayload = context.queueAddMessageWithHarvester(descriptors[0], mainAccountPublicKey);

			// - max unlocked accounts is 10 and initially there was one account, so only first 9 will be added
			if (i < 9)
				expectedEncryptedPayloads.insert(encryptedPayload);

			context.update();
		}

		// Assert:
		EXPECT_EQ(10u, context.numUnlockedAccounts());
		context.assertHarvesterFileRecords(expectedEncryptedPayloads);
	}

	// endregion

	// region update - save + remove (single account)

	TEST(TEST_CLASS, UpdateSavesAddedAccount) {
		// Arrange: add account to cache
		TestContext context;
		auto descriptors = test::GenerateRandomAccountDescriptors(1);
		auto mainAccountPublicKey = context.addEnabledAccount(descriptors[0]);

		// - prepare message
		auto encryptedPayload = context.queueAddMessageWithHarvester(descriptors[0], mainAccountPublicKey);

		// Sanity:
		EXPECT_EQ(1u, context.numUnlockedAccounts());

		// Act:
		context.update();

		// Assert: message was consumed, added to unlocked accounts and added to unlocked harvesters file
		EXPECT_EQ(2u, context.numUnlockedAccounts());
		context.assertHarvesterFileRecords({ encryptedPayload });
	}

	TEST(TEST_CLASS, UpdateDoesNotSaveRemovedAccount) {
		// Arrange: add account to cache
		TestContext context;
		auto descriptors = test::GenerateRandomAccountDescriptors(1);
		auto mainAccountPublicKey = context.addEnabledAccount(descriptors[0]);

		// - prepare and process message
		auto encryptedPayload = context.queueAddMessageWithHarvester(descriptors[0], mainAccountPublicKey);
		context.update();

		// Sanity:
		EXPECT_EQ(2u, context.numUnlockedAccounts());

		// - prepare removal message
		context.queueRemoveMessage(encryptedPayload, mainAccountPublicKey);

		// Act:
		context.update();

		// Assert: message was consumed, removed from unlocked accounts and removed unlocked harvesters file
		EXPECT_EQ(1u, context.numUnlockedAccounts());
		context.assertNoHarvesterFile();
	}

	// endregion

	// region update - save + remove (duplicate harvester)

	TEST(TEST_CLASS, UpdateDeduplicatesAddedHarvesters) {
		// Arrange: add accounts to cache
		TestContext context;
		auto descriptors = test::GenerateRandomAccountDescriptors(2);
		auto mainAccountPublicKey1 = context.addEnabledAccount(descriptors[0]);
		auto mainAccountPublicKey2 = context.addEnabledAccount(descriptors[1]);

		// - prepare and process non-consecutive messages with same harvester
		auto encryptedPayload1 = context.queueAddMessageWithHarvester(descriptors[0], mainAccountPublicKey1);
		auto encryptedPayload2 = context.queueAddMessageWithHarvester(descriptors[1], mainAccountPublicKey2);
		context.queueAddMessageWithHarvester(descriptors[0]);

		// Sanity:
		EXPECT_EQ(1u, context.numUnlockedAccounts());

		// Act:
		context.update();

		// Assert: only one message was added per harvester
		EXPECT_EQ(3u, context.numUnlockedAccounts());
		context.assertHarvesterFileRecords({ encryptedPayload1, encryptedPayload2 });
	}

	TEST(TEST_CLASS, UpdateAllowsRemovalOfSameHarvesterMultipleTimes) {
		// Arrange: add accounts to cache
		TestContext context;
		auto descriptors = test::GenerateRandomAccountDescriptors(2);
		auto mainAccountPublicKey1 = context.addEnabledAccount(descriptors[0]);
		auto mainAccountPublicKey2 = context.addEnabledAccount(descriptors[1]);

		// - prepare and process non-consecutive messages with same harvester
		auto encryptedPayload1 = context.queueAddMessageWithHarvester(descriptors[0], mainAccountPublicKey1);
		auto encryptedPayload2 = context.queueAddMessageWithHarvester(descriptors[1], mainAccountPublicKey2);
		auto encryptedPayload3 = context.queueAddMessageWithHarvester(descriptors[0], mainAccountPublicKey1);
		context.update();

		// Sanity:
		EXPECT_EQ(3u, context.numUnlockedAccounts());

		// Act: remove the third message
		context.queueRemoveMessage(encryptedPayload3, mainAccountPublicKey1);
		context.update();

		// Assert:
		// - removes the message from unlocked accounts (because harvester matches)
		// - removes the message from the storage (because announcer matches)
		EXPECT_EQ(2u, context.numUnlockedAccounts());
		context.assertHarvesterFileRecords({ encryptedPayload2 });

		// Act: remove the second message
		context.queueRemoveMessage(encryptedPayload2, mainAccountPublicKey2);
		context.update();

		// Assert:
		EXPECT_EQ(1u, context.numUnlockedAccounts());
		context.assertNoHarvesterFile();

		// Act: remove the first message
		context.queueRemoveMessage(encryptedPayload1, mainAccountPublicKey1);
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
		auto mainAccountPublicKey1 = context.addEnabledAccount(descriptors[0]);
		auto mainAccountPublicKey2 = context.addEnabledAccount(descriptors[1]);
		auto mainAccountPublicKey3 = context.addEnabledAccount(descriptors[2]);

		// - prepare and process non-consecutive messages with same announcer
		auto ephemeralKeyPair = test::GenerateKeyPair();
		auto encryptedPayload1 = context.queueAddMessageWithHarvester(ephemeralKeyPair, descriptors[0], mainAccountPublicKey1);
		auto encryptedPayload2 = context.queueAddMessageWithHarvester(descriptors[1], mainAccountPublicKey2);
		context.queueAddMessageWithHarvester(ephemeralKeyPair, descriptors[2], mainAccountPublicKey3);

		// Sanity:
		EXPECT_EQ(1u, context.numUnlockedAccounts());

		// Act:
		context.update();

		// Assert: only one message was added per announcer
		EXPECT_EQ(3u, context.numUnlockedAccounts());
		context.assertHarvesterFileRecords({ encryptedPayload1, encryptedPayload2 });
	}

	TEST(TEST_CLASS, UpdateAllowsRemovalOfSameAnnouncerMultipleTimes) {
		// Arrange: add accounts to cache
		TestContext context;
		auto descriptors = test::GenerateRandomAccountDescriptors(3);
		auto mainAccountPublicKey1 = context.addEnabledAccount(descriptors[0]);
		auto mainAccountPublicKey2 = context.addEnabledAccount(descriptors[1]);
		auto mainAccountPublicKey3 = context.addEnabledAccount(descriptors[2]);

		// - prepare and process non-consecutive messages with same announcer
		auto ephemeralKeyPair = test::GenerateKeyPair();
		auto encryptedPayload1 = context.queueAddMessageWithHarvester(ephemeralKeyPair, descriptors[0], mainAccountPublicKey1);
		auto encryptedPayload2 = context.queueAddMessageWithHarvester(descriptors[1], mainAccountPublicKey2);
		auto encryptedPayload3 = context.queueAddMessageWithHarvester(ephemeralKeyPair, descriptors[2], mainAccountPublicKey3);
		context.update();

		// Sanity:
		EXPECT_EQ(3u, context.numUnlockedAccounts());

		// Act: remove the third message
		context.queueRemoveMessage(encryptedPayload3, mainAccountPublicKey3);
		context.update();

		// Assert:
		// - removes the message from the storage (because announcer matches)
		EXPECT_EQ(3u, context.numUnlockedAccounts());
		context.assertHarvesterFileRecords({ encryptedPayload2 });

		// Act: remove the second message
		context.queueRemoveMessage(encryptedPayload2, mainAccountPublicKey2);
		context.update();

		// Assert:
		EXPECT_EQ(2u, context.numUnlockedAccounts());
		context.assertNoHarvesterFile();

		// Act: remove the first message
		context.queueRemoveMessage(encryptedPayload1, mainAccountPublicKey1);
		context.update();

		// Assert:
		// - removes the message from unlocked accounts (because harvester matches)
		EXPECT_EQ(1u, context.numUnlockedAccounts());
		context.assertNoHarvesterFile();
	}

	// endregion

	// region update - save + remove (duplicate harvester+announcer pair)

	TEST(TEST_CLASS, UpdateDeduplicatesAddedAnnouncerAndHarvesterPairs) {
		// Arrange: add accounts to cache
		TestContext context;
		auto descriptors = test::GenerateRandomAccountDescriptors(2);
		auto mainAccountPublicKey1 = context.addEnabledAccount(descriptors[0]);
		auto mainAccountPublicKey2 =context.addEnabledAccount(descriptors[1]);

		// - prepare and process non-consecutive messages with same announcer and harvester
		auto ephemeralKeyPair = test::GenerateKeyPair();
		auto encryptedPayload1 = context.queueAddMessageWithHarvester(ephemeralKeyPair, descriptors[0], mainAccountPublicKey1);
		auto encryptedPayload2 = context.queueAddMessageWithHarvester(descriptors[1], mainAccountPublicKey2);
		context.queueAddMessageWithHarvester(ephemeralKeyPair, descriptors[0], mainAccountPublicKey1);

		// Sanity:
		EXPECT_EQ(1u, context.numUnlockedAccounts());

		// Act:
		context.update();

		// Assert: only one message was added per announcer
		EXPECT_EQ(3u, context.numUnlockedAccounts());
		context.assertHarvesterFileRecords({ encryptedPayload1, encryptedPayload2 });
	}

	TEST(TEST_CLASS, UpdateAllowsRemovalOfSameAnnouncerAndHarvesterPairMultipleTimes) {
		// Arrange: add accounts to cache
		TestContext context;
		auto descriptors = test::GenerateRandomAccountDescriptors(2);
		auto mainAccountPublicKey1 = context.addEnabledAccount(descriptors[0]);
		auto mainAccountPublicKey2 = context.addEnabledAccount(descriptors[1]);

		// - prepare and process non-consecutive messages with same announcer and harvester
		auto ephemeralKeyPair = test::GenerateKeyPair();
		auto encryptedPayload1 = context.queueAddMessageWithHarvester(ephemeralKeyPair, descriptors[0], mainAccountPublicKey1);
		auto encryptedPayload2 = context.queueAddMessageWithHarvester(descriptors[1], mainAccountPublicKey2);
		auto encryptedPayload3 = context.queueAddMessageWithHarvester(ephemeralKeyPair, descriptors[0], mainAccountPublicKey1);
		context.update();

		// Sanity:
		EXPECT_EQ(3u, context.numUnlockedAccounts());

		// Act: remove the third message
		context.queueRemoveMessage(encryptedPayload3, mainAccountPublicKey1);
		context.update();

		// Assert:
		// - removes the message from unlocked accounts (because harvester matches)
		// - removes the message from the storage (because announcer matches)
		EXPECT_EQ(2u, context.numUnlockedAccounts());
		context.assertHarvesterFileRecords({ encryptedPayload2 });

		// Act: remove the second message
		context.queueRemoveMessage(encryptedPayload2, mainAccountPublicKey2);
		context.update();

		// Assert:
		EXPECT_EQ(1u, context.numUnlockedAccounts());
		context.assertNoHarvesterFile();

		// Act: remove the first message
		context.queueRemoveMessage(encryptedPayload1, mainAccountPublicKey1);
		context.update();

		// Assert: no change (remove is skipped because harvester is already locked)
		EXPECT_EQ(1u, context.numUnlockedAccounts());
		context.assertNoHarvesterFile();
	}

	// endregion
}}

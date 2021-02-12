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
			enum class SeedOptions { Address, Public_Key, Remote };

		public:
			explicit TestContext(SeedOptions seedOptions = SeedOptions::Public_Key)
					: TestContext(test::GenerateKeyPair(), test::GenerateKeyPair(), seedOptions)
			{}

		private:
			TestContext(crypto::KeyPair&& mainKeyPair, crypto::KeyPair&& remoteKeyPair, SeedOptions seedOptions)
					: m_dataDirectory(config::CatapultDataDirectory(m_directoryGuard.name()))
					, m_config(CreateBlockChainConfiguration())
					, m_cache(test::CreateEmptyCatapultCache(m_config))
					, m_unlockedAccounts(10, [](const auto&) { return 0; })
					, m_primaryMainAccountPublicKey(mainKeyPair.publicKey())
					, m_primarySigningPublicKey(SeedOptions::Remote == seedOptions ? remoteKeyPair.publicKey() : mainKeyPair.publicKey())
					, m_encryptionKeyPair(test::GenerateKeyPair())
					, m_updater(m_cache, m_unlockedAccounts, m_primarySigningPublicKey, m_encryptionKeyPair, m_dataDirectory) {
				if (SeedOptions::Remote == seedOptions) {
					auto descriptor = BlockGeneratorAccountDescriptor(std::move(remoteKeyPair), test::GenerateKeyPair());
					auto vrfPublicKey = descriptor.vrfKeyPair().publicKey();
					m_unlockedAccounts.modifier().add(std::move(descriptor));
					addAccount(m_primaryMainAccountPublicKey, m_primarySigningPublicKey, vrfPublicKey, Amount(1234));

					modifyAccount(m_primaryMainAccountPublicKey, [](auto& accountState) {
						accountState.SupplementalPublicKeys.node().unset();
					});

					return;
				}

				auto descriptor = BlockGeneratorAccountDescriptor(std::move(mainKeyPair), test::GenerateKeyPair());
				auto vrfPublicKey = descriptor.vrfKeyPair().publicKey();
				m_unlockedAccounts.modifier().add(std::move(descriptor));

				if (SeedOptions::Address == seedOptions) {
					auto address = model::PublicKeyToAddress(m_primarySigningPublicKey, m_config.Network.Identifier);
					addMainAccount(address, vrfPublicKey, Amount(1234));
				} else {
					addAccount(m_primarySigningPublicKey, vrfPublicKey, Amount(1234));
				}
			}

		public:
			size_t numUnlockedAccounts() const {
				return m_unlockedAccounts.view().size();
			}

			const Key& primaryMainAccountPublicKey() const {
				return m_primaryMainAccountPublicKey;
			}

			const Key& primarySigningPublicKey() const {
				return m_primarySigningPublicKey;
			}

			auto harvestersFilename() const {
				return m_dataDirectory.rootDir().file("harvesters.dat");
			}

			bool contains(const Key& publicKey) const {
				return m_unlockedAccounts.view().contains(publicKey);
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
						m_encryptionKeyPair.publicKey(),
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
						m_encryptionKeyPair.publicKey(),
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
				EXPECT_FALSE(std::filesystem::exists(harvestersFilename()));
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
				mainAccountStateIter.get().SupplementalPublicKeys.vrf().set(vrfPublicKey);

				m_cache.commit(Height(100));
			}

			Key addAccount(const Key& signingPublicKey, const Key& vrfPublicKey, Amount balance) {
				auto mainAccountPublicKey = test::GenerateRandomByteArray<Key>();
				addAccount(mainAccountPublicKey, signingPublicKey, vrfPublicKey, balance);
				return mainAccountPublicKey;
			}

			void addAccount(const Key& mainAccountPublicKey, const Key& signingPublicKey, const Key& vrfPublicKey, Amount balance) {
				addMainAccount(mainAccountPublicKey, vrfPublicKey, balance);

				auto delta = m_cache.createDelta();
				auto& accountStateCache = delta.sub<cache::AccountStateCache>();
				auto mainAccountStateIter = accountStateCache.find(mainAccountPublicKey);
				mainAccountStateIter.get().AccountType = state::AccountType::Main;
				mainAccountStateIter.get().SupplementalPublicKeys.linked().set(signingPublicKey);
				mainAccountStateIter.get().SupplementalPublicKeys.node().set(m_encryptionKeyPair.publicKey());

				accountStateCache.addAccount(signingPublicKey, Height(100));
				auto remoteAccountStateIter = accountStateCache.find(signingPublicKey);
				remoteAccountStateIter.get().AccountType = state::AccountType::Remote;
				remoteAccountStateIter.get().SupplementalPublicKeys.linked().set(mainAccountPublicKey);

				m_cache.commit(Height(100));
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
			Key m_primaryMainAccountPublicKey;
			Key m_primarySigningPublicKey;
			crypto::KeyPair m_encryptionKeyPair;
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

	namespace {
		void AssertUpdateDoesNotPruneValidHarvesterWithoutNodeLink(TestContext::SeedOptions seedOptions) {
			// Arrange:
			TestContext context(seedOptions);

			// Sanity:
			EXPECT_EQ(1u, context.numUnlockedAccounts());

			// Act:
			context.update();

			// Assert:
			EXPECT_EQ(1u, context.numUnlockedAccounts());
		}
	}

	TEST(TEST_CLASS, UpdateDoesNotPruneValidMainAccountWithoutNodeLink) {
		AssertUpdateDoesNotPruneValidHarvesterWithoutNodeLink(TestContext::SeedOptions::Public_Key);
	}

	TEST(TEST_CLASS, UpdateDoesNotPruneValidRemoteHarvesterWithoutNodeLink) {
		AssertUpdateDoesNotPruneValidHarvesterWithoutNodeLink(TestContext::SeedOptions::Remote);
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

	TEST(TEST_CLASS, UpdatePrunesMismatchedLinkedPublicKeyAccounts) {
		// Assert: linkedPublicKey mismatch is fatal error raised by RequireLinkedRemoteAndMainAccounts indicating state corruption
		EXPECT_THROW(AssertPruned([](auto& accountState) {
			SetRandom(accountState.SupplementalPublicKeys.linked());
		}), catapult_runtime_error);
	}

	TEST(TEST_CLASS, UpdatePrunesMismatchedNodePublicKeyAccounts) {
		AssertPruned([](auto& accountState) {
			SetRandom(accountState.SupplementalPublicKeys.node());
		});
	}

	TEST(TEST_CLASS, UpdatePrunesMismatchedVrfPublicKeyAccounts) {
		AssertPruned([](auto& accountState) {
			SetRandom(accountState.SupplementalPublicKeys.vrf());
		});
	}

	TEST(TEST_CLASS, UpdatePrunesIneligibleHarvesterAccounts) {
		AssertPruned([](auto& accountState) {
			accountState.Balances.debit(Harvesting_Mosaic_Id, accountState.Balances.get(Harvesting_Mosaic_Id));
		});
	}

	namespace {
		void AssertPrunedPrimary(const consumer<state::AccountState&>& corruptAccountState, bool expectPrune = true) {
			// Arrange: add three accounts to cache
			TestContext context(TestContext::SeedOptions::Remote);
			auto descriptors = test::GenerateRandomAccountDescriptors(3);
			auto mainAccountPublicKey1 = context.addEnabledAccount(descriptors[0]);
			auto mainAccountPublicKey2 = context.addEnabledAccount(descriptors[1]);
			auto mainAccountPublicKey3 = context.addEnabledAccount(descriptors[2]);

			auto encryptedPayload1 = context.queueAddMessageWithHarvester(descriptors[0], mainAccountPublicKey1);
			auto encryptedPayload2 = context.queueAddMessageWithHarvester(descriptors[1], mainAccountPublicKey2);
			auto encryptedPayload3 = context.queueAddMessageWithHarvester(descriptors[2], mainAccountPublicKey3);

			// - trigger initial save
			context.update();

			// Sanity:
			EXPECT_EQ(4u, context.numUnlockedAccounts());
			EXPECT_TRUE(context.contains(context.primarySigningPublicKey()));

			// Act: corrupt the primary account and resave
			context.modifyAccount(context.primaryMainAccountPublicKey(), corruptAccountState);
			context.update();

			// Assert: only eligible messages remain in unlocked accounts and unlocked harvesters file
			EXPECT_EQ(expectPrune ? 3u : 4u, context.numUnlockedAccounts());
			context.assertHarvesterFileRecords({ encryptedPayload1, encryptedPayload2, encryptedPayload3 });

			EXPECT_EQ(!expectPrune, context.contains(context.primarySigningPublicKey()));
		}
	}

	TEST(TEST_CLASS, UpdatePrunesMismatchedLinkedPublicKeyAccounts_Primary) {
		// Assert: linkedPublicKey mismatch is fatal error raised by RequireLinkedRemoteAndMainAccounts indicating state corruption
		EXPECT_THROW(AssertPrunedPrimary([](auto& accountState) {
			SetRandom(accountState.SupplementalPublicKeys.linked());
		}), catapult_runtime_error);
	}

	TEST(TEST_CLASS, UpdateDoesNotPruneMismatchedNodePublicKeyAccounts_Primary) {
		AssertPrunedPrimary([](auto& accountState) {
			SetRandom(accountState.SupplementalPublicKeys.node());
		}, false);
	}

	TEST(TEST_CLASS, UpdatePrunesMismatchedVrfPublicKeyAccounts_Primary) {
		AssertPrunedPrimary([](auto& accountState) {
			SetRandom(accountState.SupplementalPublicKeys.vrf());
		});
	}

	TEST(TEST_CLASS, UpdateDoesNotPruneIneligibleHarvesterAccounts_Primary) {
		AssertPrunedPrimary([](auto& accountState) {
			accountState.Balances.debit(Harvesting_Mosaic_Id, accountState.Balances.get(Harvesting_Mosaic_Id));
		}, false);
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

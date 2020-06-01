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

#include "catapult/cache_core/HighValueAccounts.h"
#include "tests/test/cache/AccountStateCacheTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/other/DeltaElementsTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS HighValueAccountsTests

	// region test utils

	namespace {
		constexpr auto Harvesting_Mosaic_Id = MosaicId(9876);
		constexpr auto Min_Harvester_Balance = Amount(1'000'000);
		constexpr auto Min_Voter_Balance = Amount(2'000'000);

		constexpr Amount RelVoterAmount(int64_t delta) {
			return 0 < delta
					? Min_Voter_Balance + Amount(static_cast<uint64_t>(delta))
					: Min_Voter_Balance - Amount(static_cast<uint64_t>(-delta));
		}

		cache::AccountStateCacheTypes::Options CreateOptions() {
			auto options = test::CreateDefaultAccountStateCacheOptions(MosaicId(1111), Harvesting_Mosaic_Id);
			options.MinHarvesterBalance = Min_Harvester_Balance;
			options.MinVoterBalance = Min_Voter_Balance;
			return options;
		}

		model::AddressSet GenerateRandomAddresses(size_t count) {
			auto addresses = test::GenerateRandomAddresses(count);
			return model::AddressSet(addresses.cbegin(), addresses.cend());
		}

		void Credit(state::AccountState& accountState, Amount amount) {
			accountState.Balances.credit(Harvesting_Mosaic_Id, amount);
		}

		void Debit(state::AccountState& accountState, Amount amount) {
			accountState.Balances.debit(Harvesting_Mosaic_Id, amount);
		}

		AddressBalanceHistoryMap CreateThreeBalanceHistories() {
			return test::GenerateBalanceHistories({
				{ { { 1 } }, { { Height(2), RelVoterAmount(1) }, { Height(4), RelVoterAmount(9) }, { Height(7), RelVoterAmount(-1) } } },
				{ { { 2 } }, { { Height(2), RelVoterAmount(0) }, { Height(6), RelVoterAmount(-100) } } },
				{ { { 3 } }, { { Height(3), RelVoterAmount(9) }, { Height(4), RelVoterAmount(-9) }, { Height(7), RelVoterAmount(8) } } }
			});
		}
	}

	// endregion

	// region accounts - constructor

	TEST(TEST_CLASS, Accounts_CanCreateEmptyAccounts) {
		// Act:
		HighValueAccounts accounts;

		// Assert:
		EXPECT_TRUE(accounts.addresses().empty());

		EXPECT_TRUE(accounts.balanceHistories().empty());
	}

	TEST(TEST_CLASS, Accounts_CanCreateAroundInputs) {
		// Act:
		auto addresses = GenerateRandomAddresses(4);
		auto balanceHistories = CreateThreeBalanceHistories();
		HighValueAccounts accounts(addresses, balanceHistories);

		// Assert:
		EXPECT_EQ(addresses, accounts.addresses());

		test::AssertEqual(balanceHistories, accounts.balanceHistories());
	}

	TEST(TEST_CLASS, Accounts_CanCreateAroundMovedInputs) {
		// Act:
		auto addresses = GenerateRandomAddresses(4);
		auto addressesCopy = addresses;
		HighValueAccounts accounts(std::move(addresses), CreateThreeBalanceHistories());

		// Assert:
		EXPECT_EQ(addressesCopy, accounts.addresses());

		test::AssertEqual(CreateThreeBalanceHistories(), accounts.balanceHistories());
	}

	// endregion

	// region updater - constructor

	namespace {
		HighValueAccounts CreateAccounts(const model::AddressSet& addresses) {
			return HighValueAccounts(addresses, AddressBalanceHistoryMap());
		}
	}

	TEST(TEST_CLASS, Updater_CanCreateAroundAccounts) {
		// Act:
		auto accounts = HighValueAccounts(GenerateRandomAddresses(4), CreateThreeBalanceHistories());
		HighValueAccountsUpdater updater(CreateOptions(), accounts);

		// Assert:
		EXPECT_EQ(Height(1), updater.height());

		EXPECT_EQ(accounts.addresses(), updater.addresses());
		EXPECT_TRUE(updater.removedAddresses().empty());

		test::AssertEqual(accounts.balanceHistories(), updater.balanceHistories());
	}

	// endregion

	// region updater - setHeight

	TEST(TEST_CLASS, Updater_CanSetHeight) {
		// Arrange:
		auto accounts = CreateAccounts(GenerateRandomAddresses(3));
		HighValueAccountsUpdater updater(CreateOptions(), accounts);

		// Act:
		updater.setHeight(Height(7));

		// Assert:
		EXPECT_EQ(Height(7), updater.height());

		EXPECT_EQ(accounts.addresses(), updater.addresses());
		EXPECT_TRUE(updater.removedAddresses().empty());

		EXPECT_TRUE(updater.balanceHistories().empty());
	}

	// endregion

	// region updater - harvester eligible accounts

	namespace {
		using MemorySetType = AccountStateCacheTypes::PrimaryTypes::BaseSetDeltaType::SetType::MemorySetType;

		std::vector<Amount> GetHarvesterEligibleTestBalances() {
			return {
				Amount(Min_Harvester_Balance),
				Amount(Min_Harvester_Balance - Amount(1)),
				Amount(Min_Harvester_Balance + Amount(1)),
				Amount(Min_Harvester_Balance - Amount(100'000)),
				Amount(Min_Harvester_Balance + Amount(100'000)),
				Amount(Min_Voter_Balance)
			};
		}

		void AddAccountsWithBalances(MemorySetType& set, const std::vector<Address>& addresses, const std::vector<Amount>& balances) {
			for (auto i = 0u; i < balances.size(); ++i) {
				auto accountState = state::AccountState(addresses[i], Height(1));
				accountState.Balances.credit(Harvesting_Mosaic_Id, balances[i]);
				set.emplace(accountState.Address, accountState);
			}
		}

		std::vector<Address> AddAccountsWithBalances(MemorySetType& set, const std::vector<Amount>& balances) {
			auto addresses = test::GenerateRandomDataVector<Address>(balances.size());
			AddAccountsWithBalances(set, addresses, balances);
			return addresses;
		}

		model::AddressSet Pick(const std::vector<Address>& addresses, std::initializer_list<size_t> indexes) {
			model::AddressSet selectedAddresses;
			for (auto index : indexes)
				selectedAddresses.insert(addresses[index]);

			return selectedAddresses;
		}

		auto& SelectAdded(test::DeltaElementsTestUtils::Wrapper<MemorySetType>& deltas) {
			return deltas.Added;
		}

		auto& SelectCopied(test::DeltaElementsTestUtils::Wrapper<MemorySetType>& deltas) {
			return deltas.Copied;
		}
	}

	namespace {
		template<typename TSetSelector>
		void AssertHarvesterEligibleAccountsProcessedAsAddAllNew(TSetSelector setSelector) {
			// Arrange:
			test::DeltaElementsTestUtils::Wrapper<MemorySetType> deltas;
			auto addedAddresses = AddAccountsWithBalances(setSelector(deltas), GetHarvesterEligibleTestBalances());

			auto accounts = CreateAccounts({});
			HighValueAccountsUpdater updater(CreateOptions(), accounts);

			// Act:
			updater.update(deltas.deltas());

			// Assert:
			EXPECT_EQ(Pick(addedAddresses, { 0, 2, 4, 5 }), updater.addresses());
			EXPECT_TRUE(updater.removedAddresses().empty());

			// Sanity:
			EXPECT_EQ(1u, updater.balanceHistories().size());
		}

		template<typename TSetSelector>
		void AssertHarvesterEligibleAccountsProcessedAsAddSomeExist(TSetSelector setSelector) {
			// Arrange:
			test::DeltaElementsTestUtils::Wrapper<MemorySetType> deltas;
			auto addedAddresses = AddAccountsWithBalances(setSelector(deltas), GetHarvesterEligibleTestBalances());

			auto accounts = CreateAccounts(model::AddressSet(addedAddresses.cbegin(), addedAddresses.cbegin() + 4));
			HighValueAccountsUpdater updater(CreateOptions(), accounts);

			// Act:
			updater.update(deltas.deltas());

			// Assert:
			EXPECT_EQ(Pick(addedAddresses, { 0, 2, 4, 5 }), updater.addresses());
			EXPECT_EQ(Pick(addedAddresses, { 1, 3 }), updater.removedAddresses());

			// Sanity:
			EXPECT_EQ(1u, updater.balanceHistories().size());
		}
	}

	TEST(TEST_CLASS, Updater_HarvesterEligible_CanProcessAddedWhenAllNew) {
		AssertHarvesterEligibleAccountsProcessedAsAddAllNew(SelectAdded);
	}

	TEST(TEST_CLASS, Updater_HarvesterEligible_CanProcessAddedWhenSomeExist) {
		AssertHarvesterEligibleAccountsProcessedAsAddSomeExist(SelectAdded);
	}

	TEST(TEST_CLASS, Updater_HarvesterEligible_CanProcessCopiedWhenAllNew) {
		AssertHarvesterEligibleAccountsProcessedAsAddAllNew(SelectCopied);
	}

	TEST(TEST_CLASS, Updater_HarvesterEligible_CanProcessCopiedWhenSomeExist) {
		AssertHarvesterEligibleAccountsProcessedAsAddSomeExist(SelectCopied);
	}

	TEST(TEST_CLASS, Updater_HarvesterEligible_CanProcessRemovedWhenAllNew) {
		// Arrange:
		test::DeltaElementsTestUtils::Wrapper<MemorySetType> deltas;
		auto addedAddresses = AddAccountsWithBalances(deltas.Removed, GetHarvesterEligibleTestBalances());

		auto accounts = CreateAccounts({});
		HighValueAccountsUpdater updater(CreateOptions(), accounts);

		// Act:
		updater.update(deltas.deltas());

		// Assert:
		EXPECT_TRUE(updater.addresses().empty());
		EXPECT_TRUE(updater.removedAddresses().empty());

		// Sanity:
		EXPECT_TRUE(updater.balanceHistories().empty());
	}

	TEST(TEST_CLASS, Updater_HarvesterEligible_CanProcessRemovedWhenSomeExist) {
		// Arrange:
		test::DeltaElementsTestUtils::Wrapper<MemorySetType> deltas;
		auto addedAddresses = AddAccountsWithBalances(deltas.Removed, GetHarvesterEligibleTestBalances());

		auto accounts = CreateAccounts(model::AddressSet(addedAddresses.cbegin(), addedAddresses.cbegin() + 4));
		HighValueAccountsUpdater updater(CreateOptions(), accounts);

		// Act:
		updater.update(deltas.deltas());

		// Assert:
		EXPECT_TRUE(updater.addresses().empty());
		EXPECT_EQ(Pick(addedAddresses, { 0, 1, 2, 3 }), updater.removedAddresses());

		// Sanity:
		EXPECT_TRUE(updater.balanceHistories().empty());
	}

	TEST(TEST_CLASS, Updater_HarvesterEligible_CanProcessMixedViaSingleUpdate) {
		// Arrange: add seven [5 match {0, 2, 4, 5, 6}]
		test::DeltaElementsTestUtils::Wrapper<MemorySetType> deltas;
		auto addedAddresses = AddAccountsWithBalances(deltas.Added, {
			Amount(1'100'000), Amount(900'000), Amount(1'000'000), Amount(800'000), Amount(1'200'000), Amount(1'400'000), Amount(1'300'000)
		});

		// - modify four [5 match {0, 1, 3, 4, 5}]
		Credit(deltas.Copied.insert(*deltas.Added.find(addedAddresses[1])).first->second, Amount(100'000));
		Debit(deltas.Copied.insert(*deltas.Added.find(addedAddresses[2])).first->second, Amount(1));
		Credit(deltas.Copied.insert(*deltas.Added.find(addedAddresses[3])).first->second, Amount(300'000));
		Debit(deltas.Copied.insert(*deltas.Added.find(addedAddresses[6])).first->second, Amount(300'001));

		// - delete two [3 match {0, 4, 5}]
		deltas.Removed.insert(*deltas.Added.find(addedAddresses[1]));
		deltas.Removed.insert(*deltas.Added.find(addedAddresses[3]));

		auto accounts = CreateAccounts(model::AddressSet(addedAddresses.cbegin(), addedAddresses.cbegin() + 3));
		HighValueAccountsUpdater updater(CreateOptions(), accounts);

		// Act:
		updater.update(deltas.deltas());

		// Assert:
		EXPECT_EQ(Pick(addedAddresses, { 0, 4, 5 }), updater.addresses());
		EXPECT_EQ(Pick(addedAddresses, { 1, 2 }), updater.removedAddresses());

		// Sanity:
		EXPECT_TRUE(updater.balanceHistories().empty());
	}

	TEST(TEST_CLASS, Updater_HarvesterEligible_CanProcessMixedViaMultipleUpdates) {
		// Arrange: add seven [5 match {0, 2, 4, 5, 6}]
		test::DeltaElementsTestUtils::Wrapper<MemorySetType> deltas;
		auto addedAddresses = AddAccountsWithBalances(deltas.Added, {
			Amount(1'100'000), Amount(900'000), Amount(1'000'000), Amount(800'000), Amount(1'200'000), Amount(1'400'000), Amount(1'300'000)
		});

		auto accounts = CreateAccounts(model::AddressSet(addedAddresses.cbegin(), addedAddresses.cbegin() + 3));
		HighValueAccountsUpdater updater(CreateOptions(), accounts);

		// Act:
		updater.update(deltas.deltas());

		// - modify four [5 match {0, 1, 3, 4, 5}]
		Credit(deltas.Copied.insert(*deltas.Added.find(addedAddresses[1])).first->second, Amount(100'000));
		Debit(deltas.Copied.insert(*deltas.Added.find(addedAddresses[2])).first->second, Amount(1));
		Credit(deltas.Copied.insert(*deltas.Added.find(addedAddresses[3])).first->second, Amount(300'000));
		Debit(deltas.Copied.insert(*deltas.Added.find(addedAddresses[6])).first->second, Amount(300'001));
		updater.update(deltas.deltas());

		// - delete two [3 match {0, 4, 5}]
		deltas.Removed.insert(*deltas.Added.find(addedAddresses[1]));
		deltas.Removed.insert(*deltas.Added.find(addedAddresses[3]));
		updater.update(deltas.deltas());

		// Assert:
		EXPECT_EQ(Pick(addedAddresses, { 0, 4, 5 }), updater.addresses());
		EXPECT_EQ(Pick(addedAddresses, { 1, 2 }), updater.removedAddresses());

		// Sanity:
		EXPECT_TRUE(updater.balanceHistories().empty());
	}

	// endregion

	// region updater - harvester eligible accounts

	namespace {
		std::vector<Amount> GetVoterEligibleTestBalances() {
			return {
				Amount(Min_Voter_Balance - Amount(1)),
				Amount(Min_Voter_Balance),
				Amount(Min_Voter_Balance - Amount(100'000)),
				Amount(Min_Voter_Balance + Amount(100'000))
			};
		}
	}

	namespace {
		template<typename TSetSelector>
		void AssertVoterEligibleAccountsProcessedAsAddAllNew(TSetSelector setSelector) {
			// Arrange:
			test::DeltaElementsTestUtils::Wrapper<MemorySetType> deltas;
			auto addedAddresses = AddAccountsWithBalances(setSelector(deltas), GetVoterEligibleTestBalances());

			auto accounts = CreateAccounts({});
			HighValueAccountsUpdater updater(CreateOptions(), accounts);

			// Act:
			updater.setHeight(Height(3));
			updater.update(deltas.deltas());

			// Assert:
			auto expectedBalanceHistories = test::GenerateBalanceHistories({
				{ addedAddresses[1], { { Height(3), Min_Voter_Balance } } },
				{ addedAddresses[3], { { Height(3), Min_Voter_Balance + Amount(100'000) } } }
			});

			test::AssertEqual(expectedBalanceHistories, updater.balanceHistories());

			// Sanity:
			EXPECT_EQ(4u, updater.addresses().size());
			EXPECT_TRUE(updater.removedAddresses().empty());
		}

		template<typename TSetSelector>
		void AssertVoterEligibleAccountsProcessedAsAddAllIncreaseAboveThreshold(TSetSelector setSelector) {
			// Arrange:
			auto accounts = CreateAccounts({});
			HighValueAccountsUpdater updater(CreateOptions(), accounts);

			// - seed 2/4 voter eligible accounts
			std::vector<Address> addedAddresses;
			{
				test::DeltaElementsTestUtils::Wrapper<MemorySetType> deltas;
				addedAddresses = AddAccountsWithBalances(setSelector(deltas), GetVoterEligibleTestBalances());

				updater.setHeight(Height(3));
				updater.update(deltas.deltas());
			}

			// Act: modify all to be voter eligible
			{
				test::DeltaElementsTestUtils::Wrapper<MemorySetType> deltas;
				AddAccountsWithBalances(setSelector(deltas), addedAddresses, {
					Min_Voter_Balance + Amount(1),
					Min_Voter_Balance + Amount(2),
					Min_Voter_Balance + Amount(3),
					Min_Voter_Balance + Amount(4)
				});

				updater.setHeight(Height(5));
				updater.update(deltas.deltas());
			}

			// Assert:
			auto expectedBalanceHistories = test::GenerateBalanceHistories({
				{ addedAddresses[0], { { Height(5), Min_Voter_Balance + Amount(1) } } },
				{ addedAddresses[1], { { Height(3), Min_Voter_Balance }, { Height(5), Min_Voter_Balance + Amount(2) } } },
				{ addedAddresses[2], { { Height(5), Min_Voter_Balance + Amount(3) } } },
				{ addedAddresses[3], { { Height(3), Min_Voter_Balance + Amount(100'000) }, { Height(5), Min_Voter_Balance + Amount(4) } } }
			});

			test::AssertEqual(expectedBalanceHistories, updater.balanceHistories());

			// Sanity:
			EXPECT_EQ(4u, updater.addresses().size());
			EXPECT_TRUE(updater.removedAddresses().empty());
		}

		template<typename TSetSelector>
		void AssertVoterEligibleAccountsProcessedAsAddAllDecreaseBelowThreshold(TSetSelector setSelector) {
			// Arrange:
			auto accounts = CreateAccounts({});
			HighValueAccountsUpdater updater(CreateOptions(), accounts);

			// - seed 2/4 voter eligible accounts
			std::vector<Address> addedAddresses;
			{
				test::DeltaElementsTestUtils::Wrapper<MemorySetType> deltas;
				addedAddresses = AddAccountsWithBalances(setSelector(deltas), GetVoterEligibleTestBalances());

				updater.setHeight(Height(3));
				updater.update(deltas.deltas());
			}

			// Act: modify all to be voter ineligible
			{
				test::DeltaElementsTestUtils::Wrapper<MemorySetType> deltas;
				AddAccountsWithBalances(setSelector(deltas), addedAddresses, {
					Min_Voter_Balance - Amount(1),
					Min_Voter_Balance - Amount(2),
					Min_Voter_Balance - Amount(3),
					Min_Voter_Balance - Amount(4)
				});

				updater.setHeight(Height(5));
				updater.update(deltas.deltas());
			}

			// Assert:
			auto expectedBalanceHistories = test::GenerateBalanceHistories({
				{ addedAddresses[1], { { Height(3), Min_Voter_Balance }, { Height(5), Min_Voter_Balance - Amount(2) } } },
				{ addedAddresses[3], { { Height(3), Min_Voter_Balance + Amount(100'000) }, { Height(5), Min_Voter_Balance - Amount(4) } } }
			});

			test::AssertEqual(expectedBalanceHistories, updater.balanceHistories());

			// Sanity:
			EXPECT_EQ(4u, updater.addresses().size());
			EXPECT_TRUE(updater.removedAddresses().empty());
		}
	}

	TEST(TEST_CLASS, Updater_VoterEligible_CanProcessAddedWhenAllNew) {
		AssertVoterEligibleAccountsProcessedAsAddAllNew(SelectAdded);
	}

	TEST(TEST_CLASS, Updater_VoterEligible_CanProcessAddedWhenAllIncreaseAboveThreshold) {
		AssertVoterEligibleAccountsProcessedAsAddAllIncreaseAboveThreshold(SelectAdded);
	}

	TEST(TEST_CLASS, Updater_VoterEligible_CanProcessAddedWhenAllDecreaseBelowThreshold) {
		AssertVoterEligibleAccountsProcessedAsAddAllDecreaseBelowThreshold(SelectAdded);
	}

	TEST(TEST_CLASS, Updater_VoterEligible_CanProcessCopiedWhenAllNew) {
		AssertVoterEligibleAccountsProcessedAsAddAllNew(SelectCopied);
	}

	TEST(TEST_CLASS, Updater_VoterEligible_CanProcessCopiedWhenAllIncreaseAboveThreshold) {
		AssertVoterEligibleAccountsProcessedAsAddAllIncreaseAboveThreshold(SelectCopied);
	}

	TEST(TEST_CLASS, Updater_VoterEligible_CanProcessCopiedWhenAllDecreaseBelowThreshold) {
		AssertVoterEligibleAccountsProcessedAsAddAllDecreaseBelowThreshold(SelectCopied);
	}

	TEST(TEST_CLASS, Updater_VoterEligible_CanProcessRemovedWhenAllNew) {
		// Arrange:
		test::DeltaElementsTestUtils::Wrapper<MemorySetType> deltas;
		auto addedAddresses = AddAccountsWithBalances(deltas.Removed, GetVoterEligibleTestBalances());

		auto accounts = CreateAccounts({});
		HighValueAccountsUpdater updater(CreateOptions(), accounts);

		// Act:
		updater.setHeight(Height(3));
		updater.update(deltas.deltas());

		// Assert:
		EXPECT_TRUE(updater.balanceHistories().empty());

		// Sanity:
		EXPECT_TRUE(updater.addresses().empty());
		EXPECT_TRUE(updater.removedAddresses().empty());
	}

	TEST(TEST_CLASS, Updater_VoterEligible_CanProcessRemovedWhenSomeExist) {
		// Arrange:
		auto accounts = CreateAccounts({});
		HighValueAccountsUpdater updater(CreateOptions(), accounts);

		// - seed 2/4 voter eligible accounts
		std::vector<Address> addedAddresses;
		{
			test::DeltaElementsTestUtils::Wrapper<MemorySetType> deltas;
			addedAddresses = AddAccountsWithBalances(deltas.Added, GetVoterEligibleTestBalances());

			updater.setHeight(Height(3));
			updater.update(deltas.deltas());
		}

		// Act: modify all to be voter eligible (but removed)
		{
			test::DeltaElementsTestUtils::Wrapper<MemorySetType> deltas;
			AddAccountsWithBalances(deltas.Removed, addedAddresses, {
				Min_Voter_Balance + Amount(1),
				Min_Voter_Balance + Amount(2),
				Min_Voter_Balance + Amount(3),
				Min_Voter_Balance + Amount(4)
			});

			updater.setHeight(Height(5));
			updater.update(deltas.deltas());
		}

		// Assert:
		auto expectedBalanceHistories = test::GenerateBalanceHistories({
			{ addedAddresses[1], { { Height(3), Min_Voter_Balance }, { Height(5), Amount() } } },
			{ addedAddresses[3], { { Height(3), Min_Voter_Balance + Amount(100'000) }, { Height(5), Amount() } } }
		});

		test::AssertEqual(expectedBalanceHistories, updater.balanceHistories());

		// Sanity:
		EXPECT_TRUE(updater.addresses().empty());
		EXPECT_TRUE(updater.removedAddresses().empty());
	}

	TEST(TEST_CLASS, Updater_VoterEligible_CanProcessMixedViaSingleUpdate) {
		// Arrange: add seven [5 match {0, 2, 4, 5, 6}]
		test::DeltaElementsTestUtils::Wrapper<MemorySetType> deltas;
		auto addedAddresses = AddAccountsWithBalances(deltas.Added, {
			Amount(2'100'000), Amount(900'000), Amount(2'000'000), Amount(800'000), Amount(2'200'000), Amount(2'400'000), Amount(2'300'000)
		});

		// - modify four [5 match {0, 1, 3, 4, 5}]
		Credit(deltas.Copied.insert(*deltas.Added.find(addedAddresses[1])).first->second, Amount(1'200'000));
		Debit(deltas.Copied.insert(*deltas.Added.find(addedAddresses[2])).first->second, Amount(1));
		Credit(deltas.Copied.insert(*deltas.Added.find(addedAddresses[3])).first->second, Amount(1'250'000));
		Debit(deltas.Copied.insert(*deltas.Added.find(addedAddresses[6])).first->second, Amount(300'001));

		// - delete two [3 match {0, 3, 5}]
		deltas.Removed.insert(*deltas.Added.find(addedAddresses[1]));
		deltas.Removed.insert(*deltas.Added.find(addedAddresses[4]));

		auto accounts = CreateAccounts({});
		HighValueAccountsUpdater updater(CreateOptions(), accounts);

		// Act:
		updater.setHeight(Height(3));
		updater.update(deltas.deltas());

		// Assert:
		auto expectedBalanceHistories = test::GenerateBalanceHistories({
			{ addedAddresses[0], { { Height(3), Amount(2'100'000) } } },
			{ addedAddresses[3], { { Height(3), Amount(2'050'000) } } },
			{ addedAddresses[5], { { Height(3), Amount(2'400'000) } } }
		});

		test::AssertEqual(expectedBalanceHistories, updater.balanceHistories());

		// Sanity:
		EXPECT_EQ(5u, updater.addresses().size());
		EXPECT_TRUE(updater.removedAddresses().empty());
	}

	TEST(TEST_CLASS, Updater_VoterEligible_CanProcessMixedViaMultipleUpdates) {
		// Arrange: add seven [5 match {0, 2, 4, 5, 6}]
		test::DeltaElementsTestUtils::Wrapper<MemorySetType> deltas;
		auto addedAddresses = AddAccountsWithBalances(deltas.Added, {
			Amount(2'100'000), Amount(900'000), Amount(2'000'000), Amount(800'000), Amount(2'200'000), Amount(2'400'000), Amount(2'300'000)
		});

		auto accounts = CreateAccounts({});
		HighValueAccountsUpdater updater(CreateOptions(), accounts);

		// Act:
		updater.setHeight(Height(3));
		updater.update(deltas.deltas());

		// - modify four [5 match {0, 1, 3, 4, 5}]
		Credit(deltas.Copied.insert(*deltas.Added.find(addedAddresses[1])).first->second, Amount(1'200'000));
		Debit(deltas.Copied.insert(*deltas.Added.find(addedAddresses[2])).first->second, Amount(1));
		Credit(deltas.Copied.insert(*deltas.Added.find(addedAddresses[3])).first->second, Amount(1'250'000));
		Debit(deltas.Copied.insert(*deltas.Added.find(addedAddresses[6])).first->second, Amount(300'001));

		updater.setHeight(Height(4));
		updater.update(deltas.deltas());

		// - delete two [3 match {0, 3, 5}]
		deltas.Removed.insert(*deltas.Added.find(addedAddresses[1]));
		deltas.Removed.insert(*deltas.Added.find(addedAddresses[4]));

		updater.setHeight(Height(5));
		updater.update(deltas.deltas());

		// Assert:
		auto expectedBalanceHistories = test::GenerateBalanceHistories({
			{ addedAddresses[0], { { Height(3), Amount(2'100'000) } } },
			{ addedAddresses[1], { { Height(4), Amount(2'100'000) }, { Height(5), Amount() } } },
			{ addedAddresses[2], { { Height(3), Amount(2'000'000) }, { Height(4), Amount(1'999'999) } } },
			{ addedAddresses[3], { { Height(4), Amount(2'050'000) } } },
			{ addedAddresses[4], { { Height(3), Amount(2'200'000) }, { Height(5), Amount() } } },
			{ addedAddresses[5], { { Height(3), Amount(2'400'000) } } },
			{ addedAddresses[6], { { Height(3), Amount(2'300'000) }, { Height(4), Amount(1'999'999) } } }
		});

		test::AssertEqual(expectedBalanceHistories, updater.balanceHistories());

		// Sanity:
		EXPECT_EQ(5u, updater.addresses().size());
		EXPECT_TRUE(updater.removedAddresses().empty());
	}

	// endregion

	// region updater - prune

	namespace {
		template<typename TAction>
		void RunPruneTest(TAction action) {
			// Arrange: add seven [5 match {0, 2, 4, 5, 6}]
			test::DeltaElementsTestUtils::Wrapper<MemorySetType> deltas;
			auto addedAddresses = AddAccountsWithBalances(deltas.Added, {
				Amount(2'100'000),
				Amount(900'000),
				Amount(2'000'001),
				Amount(800'000),
				Amount(2'200'000),
				Amount(2'400'000),
				Amount(2'300'000)
			});

			auto accounts = CreateAccounts({});
			HighValueAccountsUpdater updater(CreateOptions(), accounts);

			// Act:
			updater.setHeight(Height(3));
			updater.update(deltas.deltas());

			// - modify four [5 match {0, 1, 3, 4, 5}]
			Credit(deltas.Copied.insert(*deltas.Added.find(addedAddresses[1])).first->second, Amount(1'200'000));
			Debit(deltas.Copied.insert(*deltas.Added.find(addedAddresses[2])).first->second, Amount(2));
			Credit(deltas.Copied.insert(*deltas.Added.find(addedAddresses[3])).first->second, Amount(1'250'000));
			Debit(deltas.Copied.insert(*deltas.Added.find(addedAddresses[6])).first->second, Amount(300'001));

			updater.setHeight(Height(4));
			updater.update(deltas.deltas());

			// - modify two [5 match {0, 1, 2, 4, 5}]
			Credit(deltas.Copied.insert(*deltas.Added.find(addedAddresses[2])).first->second, Amount(1));
			Debit(deltas.Copied.insert(*deltas.Added.find(addedAddresses[3])).first->second, Amount(2'000'000));

			updater.setHeight(Height(5));
			updater.update(deltas.deltas());

			// - delete two [3 match {0, 2, 5}]
			deltas.Removed.insert(*deltas.Added.find(addedAddresses[1]));
			deltas.Removed.insert(*deltas.Added.find(addedAddresses[4]));

			updater.setHeight(Height(6));
			updater.update(deltas.deltas());

			// Assert:
			action(updater, addedAddresses);
		}

		void AssertPruneHasNoEffect(Height height) {
			// Act:
			RunPruneTest([height](auto& updater, const auto& addedAddresses) {
				updater.prune(height);

				// Assert:
				auto expectedBalanceHistories = test::GenerateBalanceHistories({
					{ addedAddresses[0], { { Height(3), Amount(2'100'000) } } },
					{ addedAddresses[1], { { Height(4), Amount(2'100'000) }, { Height(6), Amount() } } },
					{
						addedAddresses[2],
						{ { Height(3), Amount(2'000'001) }, { Height(4), Amount(1'999'999) }, { Height(5), Amount(2'000'000) } }
					},
					{ addedAddresses[3], { { Height(4), Amount(2'050'000) }, { Height(5), Amount(50'000) } } },
					{ addedAddresses[4], { { Height(3), Amount(2'200'000) }, { Height(6), Amount() } } },
					{ addedAddresses[5], { { Height(3), Amount(2'400'000) } } },
					{ addedAddresses[6], { { Height(3), Amount(2'300'000) }, { Height(4), Amount(1'999'999) } } }
				});

				test::AssertEqual(expectedBalanceHistories, updater.balanceHistories());

				// Sanity:
				EXPECT_EQ(4u, updater.addresses().size());
				EXPECT_TRUE(updater.removedAddresses().empty());
			});
		}
	}

	TEST(TEST_CLASS, Updater_PruneDoesNotAffectHistoriesWithHeightLargerThanPruneHeight) {
		AssertPruneHasNoEffect(Height(2));
	}

	TEST(TEST_CLASS, Updater_PruneDoesNotAffectHistoriesWithHeightEqualToPruneHeight) {
		AssertPruneHasNoEffect(Height(3));
	}

	TEST(TEST_CLASS, Updater_PruneDoesAffectHistoriesWithHeightLessThanPruneHeight_Height4) {
		// Act:
		RunPruneTest([](auto& updater, const auto& addedAddresses) {
			updater.prune(Height(4));

			// Assert:
			auto expectedBalanceHistories = test::GenerateBalanceHistories({
				{ addedAddresses[0], { { Height(4), Amount(2'100'000) } } },
				{ addedAddresses[1], { { Height(4), Amount(2'100'000) }, { Height(6), Amount() } } },
				{ addedAddresses[2], { { Height(4), Amount(1'999'999) }, { Height(5), Amount(2'000'000) } } },
				{ addedAddresses[3], { { Height(4), Amount(2'050'000) }, { Height(5), Amount(50'000) } } },
				{ addedAddresses[4], { { Height(4), Amount(2'200'000) }, { Height(6), Amount() } } },
				{ addedAddresses[5], { { Height(4), Amount(2'400'000) } } }
			});

			test::AssertEqual(expectedBalanceHistories, updater.balanceHistories());

			// Sanity: pruning doesn't affect addresses
			EXPECT_EQ(4u, updater.addresses().size());
			EXPECT_TRUE(updater.removedAddresses().empty());
		});
	}

	TEST(TEST_CLASS, Updater_PruneDoesAffectHistoriesWithHeightLessThanPruneHeight_Height5) {
		// Act:
		RunPruneTest([](auto& updater, const auto& addedAddresses) {
			updater.prune(Height(5));

			// Assert:
			auto expectedBalanceHistories = test::GenerateBalanceHistories({
				{ addedAddresses[0], { { Height(5), Amount(2'100'000) } } },
				{ addedAddresses[1], { { Height(5), Amount(2'100'000) }, { Height(6), Amount() } } },
				{ addedAddresses[2], { { Height(5), Amount(2'000'000) } } },
				{ addedAddresses[4], { { Height(5), Amount(2'200'000) }, { Height(6), Amount() } } },
				{ addedAddresses[5], { { Height(5), Amount(2'400'000) } } }
			});

			test::AssertEqual(expectedBalanceHistories, updater.balanceHistories());

			// Sanity: pruning doesn't affect addresses
			EXPECT_EQ(4u, updater.addresses().size());
			EXPECT_TRUE(updater.removedAddresses().empty());
		});
	}

	namespace {
		void AssertPruneTerminal(Height height) {
			// Act:
			RunPruneTest([height](auto& updater, const auto& addedAddresses) {
				updater.prune(height);

				// Assert:
				auto expectedBalanceHistories = test::GenerateBalanceHistories({
					{ addedAddresses[0], { { height, Amount(2'100'000) } } },
					{ addedAddresses[2], { { height, Amount(2'000'000) } } },
					{ addedAddresses[5], { { height, Amount(2'400'000) } } }
				});

				test::AssertEqual(expectedBalanceHistories, updater.balanceHistories());

				// Sanity: pruning doesn't affect addresses
				EXPECT_EQ(4u, updater.addresses().size());
				EXPECT_TRUE(updater.removedAddresses().empty());
			});
		}
	}

	TEST(TEST_CLASS, Updater_PruneDoesAffectHistoriesWithHeightLessThanPruneHeight_Height6) {
		AssertPruneTerminal(Height(6));
	}

	TEST(TEST_CLASS, Updater_PruneCanNeverPruneHistoriesHavingAtLeastMinBalance) {
		AssertPruneTerminal(Height(101));
	}

	// endregion

	// region updater - detachAccounts

	TEST(TEST_CLASS, Updater_DetachAccountsReturnsExpectedHighValueAccounts) {
		// Arrange:
		test::DeltaElementsTestUtils::Wrapper<MemorySetType> deltas;
		auto addedAddresses = AddAccountsWithBalances(deltas.Added, GetHarvesterEligibleTestBalances());

		auto originalAccounts = HighValueAccounts(
				model::AddressSet(addedAddresses.cbegin(), addedAddresses.cbegin() + 3),
				CreateThreeBalanceHistories());
		HighValueAccountsUpdater updater(CreateOptions(), originalAccounts);
		updater.setHeight(Height(3));
		updater.update(deltas.deltas());

		// Act:
		auto accounts = updater.detachAccounts();

		// Assert:
		EXPECT_EQ(Pick(addedAddresses, { 0, 2, 4, 5 }), accounts.addresses());

		// - notice that GetHarvesterEligibleTestBalances includes one account with min voter balance
		auto expectedBalanceHistories = CreateThreeBalanceHistories();
		expectedBalanceHistories.emplace(addedAddresses[5], test::CreateBalanceHistory({ { Height(3), Min_Voter_Balance } }));
		test::AssertEqual(expectedBalanceHistories, accounts.balanceHistories());

		// - updater is cleared
		EXPECT_TRUE(updater.addresses().empty());
		EXPECT_TRUE(updater.removedAddresses().empty());

		EXPECT_TRUE(updater.balanceHistories().empty());
	}

	// endregion
}}

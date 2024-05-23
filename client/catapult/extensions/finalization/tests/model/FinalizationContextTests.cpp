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

#include "finalization/src/model/FinalizationContext.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/Address.h"
#include "tests/test/cache/AccountStateCacheTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS FinalizationContextTests

	namespace {
		// region test utils

		using Epoch = FinalizationEpoch;

		constexpr auto Harvesting_Mosaic_Id = MosaicId(9876);

		cache::AccountStateCacheTypes::Options CreateOptions() {
			auto options = test::CreateDefaultAccountStateCacheOptions(MosaicId(1111), Harvesting_Mosaic_Id);
			options.MinVoterBalance = Amount(2'000'000);
			return options;
		}

		finalization::FinalizationConfiguration CreateConfigurationWithSize(uint32_t size) {
			auto config = finalization::FinalizationConfiguration::Uninitialized();
			config.Size = size;
			return config;
		}

		struct ExtendedFinalizationAccountView : public FinalizationAccountView {
			catapult::Address Address;
			VotingKey VotingPublicKey1;
			VotingKey VotingPublicKey2;
		};

		ExtendedFinalizationAccountView GenerateRandomAccountView(Amount balance) {
			ExtendedFinalizationAccountView accountView;
			accountView.Weight = balance;
			test::FillWithRandomData(accountView.Address);
			test::FillWithRandomData(accountView.VotingPublicKey1);
			test::FillWithRandomData(accountView.VotingPublicKey2);
			return accountView;
		}

		std::vector<ExtendedFinalizationAccountView> AddAccountsWithBalances(
				cache::AccountStateCache& cache,
				Height height,
				const std::vector<Amount>& balances) {
			std::vector<ExtendedFinalizationAccountView> accountViews;

			auto delta = cache.createDelta();
			for (auto balance : balances) {
				auto accountView = GenerateRandomAccountView(balance);
				accountViews.emplace_back(accountView);

				delta->addAccount(accountView.Address, height);
				auto& accountState = delta->find(accountView.Address).get();
				accountState.SupplementalPublicKeys.voting().add({ accountView.VotingPublicKey1, Epoch(1), Epoch(100) });
				accountState.SupplementalPublicKeys.voting().add({ accountView.VotingPublicKey2, Epoch(151), Epoch(200) });
				accountState.Balances.credit(Harvesting_Mosaic_Id, balance);
			}

			delta->updateHighValueAccounts(height);
			cache.commit();

			return accountViews;
		}

		void ModifyAccount(
				cache::AccountStateCache& cache,
				Height height,
				const Address& address,
				const consumer<state::AccountState&>& modifier) {
			auto delta = cache.createDelta();

			modifier(delta->find(address).get());

			delta->updateHighValueAccounts(height);
			cache.commit();
		}

		consumer<state::AccountState&> SetBalanceModifier(Amount balance) {
			return [balance](auto& accountState) {
				auto& balances = accountState.Balances;
				balances.debit(Harvesting_Mosaic_Id, balances.get(Harvesting_Mosaic_Id));
				balances.credit(Harvesting_Mosaic_Id, balance);
			};
		}

		consumer<state::AccountState&> AppendVotingPublicKeyModifier(Epoch startEpoch, Epoch endEpoch) {
			return [startEpoch, endEpoch](auto& accountState) {
				accountState.SupplementalPublicKeys.voting().add({ test::GenerateRandomByteArray<VotingKey>(), startEpoch, endEpoch });
			};
		}

		template<typename TAction>
		void RunNineAccountTest(TAction action) {
			// Arrange:
			auto generationHash = test::GenerateRandomByteArray<GenerationHash>();
			auto config = CreateConfigurationWithSize(9876);

			cache::AccountStateCache cache(cache::CacheConfiguration(), CreateOptions());
			auto accountViews1 = AddAccountsWithBalances(cache, Height(122), { Amount(7'000'000), Amount(4'000'000), Amount(1'000'000) });
			auto accountViews2 = AddAccountsWithBalances(cache, Height(123), { Amount(2'000'000), Amount(1'000'000), Amount(6'000'000) });
			auto accountViews3 = AddAccountsWithBalances(cache, Height(124), { Amount(1'000'000), Amount(6'000'000), Amount(3'000'000) });

			// Act:
			FinalizationContext context(Epoch(50), Height(123), generationHash, config, *cache.createView());

			// Assert:
			action(context, generationHash, accountViews1, accountViews2, accountViews3);
		}

		// endregion
	}

	// region constructor

	TEST(TEST_CLASS, CanCreateContextAroundNoHighValueAccounts) {
		// Arrange:
		auto generationHash = test::GenerateRandomByteArray<GenerationHash>();
		auto config = CreateConfigurationWithSize(9876);

		cache::AccountStateCache cache(cache::CacheConfiguration(), CreateOptions());

		// Act:
		FinalizationContext context(Epoch(50), Height(123), generationHash, config, *cache.createView());

		// Assert:
		EXPECT_EQ(Epoch(50), context.epoch());
		EXPECT_EQ(Height(123), context.height());
		EXPECT_EQ(generationHash, context.generationHash());
		EXPECT_EQ(9876u, context.config().Size);
		EXPECT_EQ(Amount(0), context.weight());
	}

	TEST(TEST_CLASS, CanCreateContextAroundHighValueAccounts) {
		// Arrange:
		auto generationHash = test::GenerateRandomByteArray<GenerationHash>();
		auto config = CreateConfigurationWithSize(9876);

		cache::AccountStateCache cache(cache::CacheConfiguration(), CreateOptions());
		auto accountViews =
				AddAccountsWithBalances(cache, Height(123), { Amount(2'000'000), Amount(4'000'000), Amount(1'000'000), Amount(6'000'000) });

		// Act:
		FinalizationContext context(Epoch(50), Height(123), generationHash, config, *cache.createView());

		// Assert:
		EXPECT_EQ(Epoch(50), context.epoch());
		EXPECT_EQ(Height(123), context.height());
		EXPECT_EQ(generationHash, context.generationHash());
		EXPECT_EQ(9876u, context.config().Size);
		EXPECT_EQ(Amount(12'000'000), context.weight());
	}

	TEST(TEST_CLASS, CanCreateContextAroundHighValueAccounts_HeightDependent) {
		// Arrange:
		RunNineAccountTest([](const auto& context, const auto& generationHash, const auto&, const auto&, const auto&) {
			// Assert: should include accounts from both views 1 and 2 but not 3
			EXPECT_EQ(Epoch(50), context.epoch());
			EXPECT_EQ(Height(123), context.height());
			EXPECT_EQ(generationHash, context.generationHash());
			EXPECT_EQ(9876u, context.config().Size);
			EXPECT_EQ(Amount(19'000'000), context.weight());
		});
	}

	TEST(TEST_CLASS, CanCreateContextAroundHighValueAccounts_AccountTransitioningAboveMinVoterBalance) {
		// Arrange:
		auto generationHash = test::GenerateRandomByteArray<GenerationHash>();
		auto config = CreateConfigurationWithSize(9876);

		cache::AccountStateCache cache(cache::CacheConfiguration(), CreateOptions());
		auto accountViews = AddAccountsWithBalances(cache, Height(122), { Amount(7'000'000), Amount(1'000'000), Amount(4'000'000) });

		// - transition second account to voter eligible
		ModifyAccount(cache, Height(123), accountViews[1].Address, SetBalanceModifier(Amount(3'000'000)));

		// Act:
		FinalizationContext context(Epoch(50), Height(123), generationHash, config, *cache.createView());

		// Assert:
		EXPECT_EQ(Epoch(50), context.epoch());
		EXPECT_EQ(Height(123), context.height());
		EXPECT_EQ(generationHash, context.generationHash());
		EXPECT_EQ(9876u, context.config().Size);
		EXPECT_EQ(Amount(14'000'000), context.weight());

		// Sanity:
		EXPECT_EQ(Amount(3'000'000), context.lookup(accountViews[1].VotingPublicKey1).Weight);
	}

	TEST(TEST_CLASS, CanCreateContextAroundHighValueAccounts_AccountTransitioningBelowMinVoterBalance) {
		// Arrange:
		auto generationHash = test::GenerateRandomByteArray<GenerationHash>();
		auto config = CreateConfigurationWithSize(9876);

		cache::AccountStateCache cache(cache::CacheConfiguration(), CreateOptions());
		auto accountViews = AddAccountsWithBalances(cache, Height(122), { Amount(7'000'000), Amount(9'000'000), Amount(4'000'000) });

		// - transition second account to voter ineligible
		ModifyAccount(cache, Height(123), accountViews[1].Address, SetBalanceModifier(Amount(1'000'000)));

		// Act:
		FinalizationContext context(Epoch(50), Height(123), generationHash, config, *cache.createView());

		// Assert:
		EXPECT_EQ(Epoch(50), context.epoch());
		EXPECT_EQ(Height(123), context.height());
		EXPECT_EQ(generationHash, context.generationHash());
		EXPECT_EQ(9876u, context.config().Size);
		EXPECT_EQ(Amount(11'000'000), context.weight());

		// Sanity:
		EXPECT_EQ(Amount(0), context.lookup(accountViews[1].VotingPublicKey1).Weight);
	}

	TEST(TEST_CLASS, CanCreateContextAroundHighValueAccounts_AccountTransitioningExpiredVotingKey) {
		// Arrange:
		auto generationHash = test::GenerateRandomByteArray<GenerationHash>();
		auto config = CreateConfigurationWithSize(9876);

		cache::AccountStateCache cache(cache::CacheConfiguration(), CreateOptions());
		auto accountViews = AddAccountsWithBalances(cache, Height(122), { Amount(7'000'000), Amount(9'000'000), Amount(4'000'000) });

		// - add additional voting key to first and third accounts
		for (auto i : std::initializer_list<size_t>{ 0, 2 })
			ModifyAccount(cache, Height(123), accountViews[i].Address, AppendVotingPublicKeyModifier(Epoch(201), Epoch(250)));

		// Act:
		FinalizationContext context(Epoch(225), Height(123), generationHash, config, *cache.createView());

		// Assert:
		EXPECT_EQ(Epoch(225), context.epoch());
		EXPECT_EQ(Height(123), context.height());
		EXPECT_EQ(generationHash, context.generationHash());
		EXPECT_EQ(9876u, context.config().Size);
		EXPECT_EQ(Amount(11'000'000), context.weight());

		// Sanity:
		EXPECT_EQ(Amount(0), context.lookup(accountViews[1].VotingPublicKey1).Weight);
	}

	TEST(TEST_CLASS, CanCreateContextAroundHighValueAccounts_AccountTransitioningUnregisteredVotingKey) {
		// Arrange:
		auto generationHash = test::GenerateRandomByteArray<GenerationHash>();
		auto config = CreateConfigurationWithSize(9876);

		cache::AccountStateCache cache(cache::CacheConfiguration(), CreateOptions());
		auto accountViews = AddAccountsWithBalances(cache, Height(122), { Amount(7'000'000), Amount(9'000'000), Amount(4'000'000) });

		// - add additional voting key to first and third accounts
		ModifyAccount(cache, Height(123), accountViews[0].Address, AppendVotingPublicKeyModifier(Epoch(201), Epoch(250)));
		ModifyAccount(cache, Height(123), accountViews[2].Address, AppendVotingPublicKeyModifier(Epoch(230), Epoch(250)));

		// Act:
		FinalizationContext context(Epoch(225), Height(123), generationHash, config, *cache.createView());

		// Assert: although third account has voting key registered for future epoch, it doesn't have voting key for current epoch
		EXPECT_EQ(Epoch(225), context.epoch());
		EXPECT_EQ(Height(123), context.height());
		EXPECT_EQ(generationHash, context.generationHash());
		EXPECT_EQ(9876u, context.config().Size);
		EXPECT_EQ(Amount(7'000'000), context.weight());

		// Sanity:
		EXPECT_EQ(Amount(0), context.lookup(accountViews[1].VotingPublicKey1).Weight);
		EXPECT_EQ(Amount(0), context.lookup(accountViews[2].VotingPublicKey1).Weight);
	}

	namespace {
		template<typename TAction>
		void RunTreasuryReissuanceEpochTest(FinalizationEpoch epoch, FinalizationEpoch treasuryReissuanceEpoch, TAction action) {
			// Arrange:
			auto generationHash = test::GenerateRandomByteArray<GenerationHash>();
			auto config = CreateConfigurationWithSize(9876);

			cache::AccountStateCache cache(cache::CacheConfiguration(), CreateOptions());
			auto accountViews = AddAccountsWithBalances(
					cache,
					Height(123),
					{ Amount(7'000'000), Amount(9'000'000), Amount(4'000'000), Amount(5'000'000) });

			config.TreasuryReissuanceEpoch = treasuryReissuanceEpoch;
			config.TreasuryReissuanceEpochIneligibleVoterAddresses.insert(accountViews[1].Address);
			config.TreasuryReissuanceEpochIneligibleVoterAddresses.insert(test::GenerateRandomByteArray<Address>());
			config.TreasuryReissuanceEpochIneligibleVoterAddresses.insert(accountViews[3].Address);
			config.TreasuryReissuanceEpochIneligibleVoterAddresses.insert(test::GenerateRandomByteArray<Address>());

			// Act:
			FinalizationContext context(epoch, Height(123), generationHash, config, *cache.createView());

			// Assert:
			action(context, generationHash, accountViews);
		}

		void AssertTreasuryReissuanceEpochIneligibleVoterAddressesEligibleAtEpoch(FinalizationEpoch treasuryReissuanceEpoch) {
			// Arrange:
			RunTreasuryReissuanceEpochTest(
					FinalizationEpoch(50),
					treasuryReissuanceEpoch,
					[](const auto& context, const auto& generationHash, const auto&) {
						// Assert: no voting accounts are excluded
						EXPECT_EQ(Epoch(50), context.epoch());
						EXPECT_EQ(Height(123), context.height());
						EXPECT_EQ(generationHash, context.generationHash());
						EXPECT_EQ(9876u, context.config().Size);
						EXPECT_EQ(Amount(25'000'000), context.weight());
					});
		}

		void AssertTreasuryReissuanceEpochIneligibleVoterAddressesIneligibleAtEpoch(FinalizationEpoch treasuryReissuanceEpoch) {
			// Arrange:
			RunTreasuryReissuanceEpochTest(
					FinalizationEpoch(50),
					treasuryReissuanceEpoch,
					[](const auto& context, const auto& generationHash, const auto& accountViews) {
						// Assert: voting accounts 1 + 3 are excluded
						EXPECT_EQ(Epoch(50), context.epoch());
						EXPECT_EQ(Height(123), context.height());
						EXPECT_EQ(generationHash, context.generationHash());
						EXPECT_EQ(9876u, context.config().Size);
						EXPECT_EQ(Amount(11'000'000), context.weight());

						// Sanity:
						EXPECT_EQ(Amount(0), context.lookup(accountViews[1].VotingPublicKey1).Weight);
						EXPECT_EQ(Amount(0), context.lookup(accountViews[3].VotingPublicKey1).Weight);
					});
		}
	}

	TEST(TEST_CLASS, CanCreateContextAroundHighValueAccounts_TreasuryReissuanceEpochIneligibleVoterAddressesEligibleBeforeEpoch) {
		AssertTreasuryReissuanceEpochIneligibleVoterAddressesEligibleAtEpoch(FinalizationEpoch(100));
		AssertTreasuryReissuanceEpochIneligibleVoterAddressesEligibleAtEpoch(FinalizationEpoch(51));
	}

	TEST(TEST_CLASS, CanCreateContextAroundHighValueAccounts_TreasuryReissuanceEpochIneligibleVoterAddressesIneligibleAtEpoch) {
		AssertTreasuryReissuanceEpochIneligibleVoterAddressesIneligibleAtEpoch(FinalizationEpoch(50));
	}

	TEST(TEST_CLASS, CanCreateContextAroundHighValueAccounts_TreasuryReissuanceEpochIneligibleVoterAddressesIneligibleAfterEpoch) {
		AssertTreasuryReissuanceEpochIneligibleVoterAddressesIneligibleAtEpoch(FinalizationEpoch(49));
		AssertTreasuryReissuanceEpochIneligibleVoterAddressesIneligibleAtEpoch(FinalizationEpoch(25));
	}

	// endregion

	// region isEligibleVoter

	TEST(TEST_CLASS, IsEligibleVoterReturnsTrueForEligibleVotingAccounts) {
		// Arrange:
		RunNineAccountTest([](const auto& context, const auto&, const auto& accountViews1, const auto& accountViews2, const auto&) {
			// Assert:
			EXPECT_TRUE(context.isEligibleVoter(accountViews1[0].VotingPublicKey1));
			EXPECT_TRUE(context.isEligibleVoter(accountViews1[1].VotingPublicKey1));

			EXPECT_TRUE(context.isEligibleVoter(accountViews2[0].VotingPublicKey1));
			EXPECT_TRUE(context.isEligibleVoter(accountViews2[2].VotingPublicKey1));
		});
	}

	TEST(TEST_CLASS, IsEligibleVoterReturnsFalseForIneligibleVotingAccounts) {
		// Arrange:
		RunNineAccountTest(
				[](const auto& context, const auto&, const auto& accountViews1, const auto& accountViews2, const auto& accountViews3) {
					// Assert:
					EXPECT_FALSE(context.isEligibleVoter(accountViews1[2].VotingPublicKey1));
					EXPECT_FALSE(context.isEligibleVoter(accountViews2[1].VotingPublicKey1));

					for (const auto& accountView : accountViews3)
						EXPECT_FALSE(context.isEligibleVoter(accountView.VotingPublicKey1));

					for (const auto& votingPublicKey : test::GenerateRandomDataVector<VotingKey>(3))
						EXPECT_FALSE(context.isEligibleVoter(votingPublicKey));
				});
	}

	// endregion

	// region lookup

	namespace {
		void AssertEqual(const FinalizationAccountView& expected, const FinalizationAccountView& actual) {
			EXPECT_EQ(expected.Weight, actual.Weight);
		}

		void AssertZero(const FinalizationAccountView& accountView) {
			EXPECT_EQ(Amount(), accountView.Weight);
		}
	}

	TEST(TEST_CLASS, CanLookupFinalizationAccountViewsForEligibleVotingAccounts) {
		// Arrange:
		RunNineAccountTest([](const auto& context, const auto&, const auto& accountViews1, const auto& accountViews2, const auto&) {
			// Assert:
			AssertEqual(accountViews1[0], context.lookup(accountViews1[0].VotingPublicKey1));
			AssertEqual(accountViews1[1], context.lookup(accountViews1[1].VotingPublicKey1));

			AssertEqual(accountViews2[0], context.lookup(accountViews2[0].VotingPublicKey1));
			AssertEqual(accountViews2[2], context.lookup(accountViews2[2].VotingPublicKey1));
		});
	}

	TEST(TEST_CLASS, CannotLookupFinalizationAccountViewsForIneligibleVotingAccounts) {
		// Arrange:
		RunNineAccountTest(
				[](const auto& context, const auto&, const auto& accountViews1, const auto& accountViews2, const auto& accountViews3) {
					// Assert:
					AssertZero(context.lookup(accountViews1[2].VotingPublicKey1));
					AssertZero(context.lookup(accountViews2[1].VotingPublicKey1));

					for (const auto& accountView : accountViews3)
						AssertZero(context.lookup(accountView.VotingPublicKey1));

					for (const auto& votingPublicKey : test::GenerateRandomDataVector<VotingKey>(3))
						AssertZero(context.lookup(votingPublicKey));
				});
	}

	namespace {
		template<typename TAction>
		void RunLookupDependentOnFinalizationHeightTest(std::initializer_list<Epoch> epochs, TAction action) {
			// Arrange:
			auto generationHash = test::GenerateRandomByteArray<GenerationHash>();
			auto config = CreateConfigurationWithSize(9876);

			cache::AccountStateCache cache(cache::CacheConfiguration(), CreateOptions());
			auto accountViews = AddAccountsWithBalances(cache, Height(122), { Amount(7'000'000), Amount(4'000'000), Amount(1'000'000) });

			for (auto epoch : epochs) {
				FinalizationContext context(epoch, Height(123), generationHash, config, *cache.createView());

				// Act:
				auto lookupResult1 = context.lookup(accountViews[0].VotingPublicKey1);
				auto lookupResult2 = context.lookup(accountViews[0].VotingPublicKey2);

				// Assert:
				action(accountViews[0], lookupResult1, lookupResult2);
			}
		}
	}

	TEST(TEST_CLASS, LookupIsDependentOnFinalizationHeight_NoVotingPublicKeyAtEpoch) {
		// Act:
		RunLookupDependentOnFinalizationHeightTest(
				{ Epoch(0), Epoch(101), Epoch(125), Epoch(150), Epoch(201), Epoch(400) },
				[](const auto&, const auto& lookupResult1, const auto& lookupResult2) {
					// Assert:
					AssertZero(lookupResult1);
					AssertZero(lookupResult2);
				});
	}

	TEST(TEST_CLASS, LookupIsDependentOnFinalizationHeight_FirstVotingPublicKeyAtEpoch) {
		// Act:
		RunLookupDependentOnFinalizationHeightTest(
				{ Epoch(1), Epoch(50), Epoch(100) },
				[](const auto& accountView, const auto& lookupResult1, const auto& lookupResult2) {
					// Assert:
					AssertEqual(accountView, lookupResult1);
					AssertZero(lookupResult2);
				});
	}

	TEST(TEST_CLASS, LookupIsDependentOnFinalizationHeight_LastVotingPublicKeyAtEpoch) {
		// Act:
		RunLookupDependentOnFinalizationHeightTest(
				{ Epoch(151), Epoch(175), Epoch(200) },
				[](const auto& accountView, const auto& lookupResult1, const auto& lookupResult2) {
					// Assert:
					AssertZero(lookupResult1);
					AssertEqual(accountView, lookupResult2);
				});
	}

	// endregion
}}

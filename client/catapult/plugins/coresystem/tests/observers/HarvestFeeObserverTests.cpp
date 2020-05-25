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

#include "src/observers/Observers.h"
#include "catapult/model/Address.h"
#include "catapult/model/InflationCalculator.h"
#include "tests/test/cache/BalanceTransferTestUtils.h"
#include "tests/test/core/AccountStateTestUtils.h"
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/plugins/AccountObserverTestContext.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS HarvestFeeObserverTests

	DEFINE_COMMON_OBSERVER_TESTS(HarvestFee, { MosaicId(), 0, 0, Address() }, model::InflationCalculator())

	// region traits

	namespace {
		constexpr MosaicId Currency_Mosaic_Id(1234);
		constexpr Height Observer_Context_Height(555);

		struct UnlinkedAccountTraits {
			static auto AddAccount(cache::AccountStateCacheDelta& delta, const Key& publicKey, Height height) {
				delta.addAccount(publicKey, height);
				return delta.find(publicKey);
			}
		};

		struct MainAccountTraits {
			static auto AddAccount(cache::AccountStateCacheDelta& delta, const Key& publicKey, Height height) {
				// explicitly mark the account as a main account (local harvesting when remote harvesting is enabled)
				auto accountStateIter = UnlinkedAccountTraits::AddAccount(delta, publicKey, height);
				accountStateIter.get().AccountType = state::AccountType::Main;
				test::ForceSetLinkedPublicKey(accountStateIter.get(), test::GenerateRandomByteArray<Key>());
				return accountStateIter;
			}
		};

		struct RemoteAccountTraits {
			static auto AddAccount(cache::AccountStateCacheDelta& delta, const Key& publicKey, Height height) {
				// 1. add the main account with a balance
				auto mainAccountPublicKey = test::GenerateRandomByteArray<Key>();
				auto mainAccountStateIter = UnlinkedAccountTraits::AddAccount(delta, mainAccountPublicKey, height);
				mainAccountStateIter.get().AccountType = state::AccountType::Main;
				test::ForceSetLinkedPublicKey(mainAccountStateIter.get(), publicKey);

				// 2. add the remote account with specified key
				auto accountStateIter = UnlinkedAccountTraits::AddAccount(delta, publicKey, height);
				accountStateIter.get().AccountType = state::AccountType::Remote;
				test::ForceSetLinkedPublicKey(accountStateIter.get(), mainAccountPublicKey);
				return mainAccountStateIter;
			}
		};
	}

#define ACCOUNT_TYPE_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_UnlinkedAccount) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<UnlinkedAccountTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MainAccount) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MainAccountTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_RemoteAccount) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RemoteAccountTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region fee credit/debit

	namespace {
		Address ToAddress(const Key& key) {
			return model::PublicKeyToAddress(key, model::NetworkIdentifier::Zero);
		}

		void AssertReceipt(
				const Key& expectedKey,
				Amount expectedAmount,
				const model::BalanceChangeReceipt& receipt,
				const std::string& message = "") {
			ASSERT_EQ(sizeof(model::BalanceChangeReceipt), receipt.Size) << message;
			EXPECT_EQ(1u, receipt.Version) << message;
			EXPECT_EQ(model::Receipt_Type_Harvest_Fee, receipt.Type) << message;
			EXPECT_EQ(Currency_Mosaic_Id, receipt.Mosaic.MosaicId) << message;
			EXPECT_EQ(expectedAmount, receipt.Mosaic.Amount) << message;
			EXPECT_EQ(ToAddress(expectedKey), receipt.TargetAddress) << message;
		}

		template<typename TAction>
		void RunHarvestFeeObserverTest(
				NotifyMode notifyMode,
				const HarvestFeeOptions& options,
				const model::InflationCalculator& calculator,
				TAction action) {
			// Arrange:
			test::AccountObserverTestContext context(notifyMode, Observer_Context_Height);

			auto pObserver = CreateHarvestFeeObserver(options, calculator);

			// Act + Assert:
			action(context, *pObserver);
		}

		template<typename TAction>
		void RunHarvestFeeObserverTest(NotifyMode notifyMode, uint8_t harvestBeneficiaryPercentage, TAction action) {
			auto options = HarvestFeeOptions{ Currency_Mosaic_Id, harvestBeneficiaryPercentage, 0, Address() };
			RunHarvestFeeObserverTest(notifyMode, options, model::InflationCalculator(), action);
		}
	}

	ACCOUNT_TYPE_TRAITS_BASED_TEST(CommitCreditsHarvester) {
		// Arrange:
		RunHarvestFeeObserverTest(NotifyMode::Commit, 0, [](auto& context, const auto& observer) {
			auto harvester = test::GenerateRandomByteArray<Key>();
			auto& accountStateCache = context.cache().template sub<cache::AccountStateCache>();
			auto accountStateIter = TTraits::AddAccount(accountStateCache, harvester, Height(1));
			accountStateIter.get().Balances.credit(Currency_Mosaic_Id, Amount(987));

			auto notification = test::CreateBlockNotification(ToAddress(harvester));
			notification.TotalFee = Amount(123);

			// Act:
			test::ObserveNotification(observer, notification, context);

			// Assert:
			test::AssertBalances(context.cache(), accountStateIter.get().PublicKey, { { Currency_Mosaic_Id, Amount(987 + 123) } });

			// - if harvester is remote, it should have an unchanged balance
			if (harvester != accountStateIter.get().PublicKey)
				test::AssertBalances(context.cache(), harvester, {});

			// - check receipt
			auto pStatement = context.statementBuilder().build();
			ASSERT_EQ(1u, pStatement->TransactionStatements.size());
			const auto& receiptPair = *pStatement->TransactionStatements.find(model::ReceiptSource());
			ASSERT_EQ(1u, receiptPair.second.size());

			const auto& receipt = static_cast<const model::BalanceChangeReceipt&>(receiptPair.second.receiptAt(0));
			AssertReceipt(accountStateIter.get().PublicKey, Amount(123), receipt);
		});
	}

	ACCOUNT_TYPE_TRAITS_BASED_TEST(RollbackDebitsHarvester) {
		// Arrange:
		RunHarvestFeeObserverTest(NotifyMode::Rollback, 0, [](auto& context, const auto& observer) {
			auto harvester = test::GenerateRandomByteArray<Key>();
			auto& accountStateCache = context.cache().template sub<cache::AccountStateCache>();
			auto accountStateIter = TTraits::AddAccount(accountStateCache, harvester, Height(1));
			accountStateIter.get().Balances.credit(Currency_Mosaic_Id, Amount(987 + 123));

			auto notification = test::CreateBlockNotification(ToAddress(harvester));
			notification.TotalFee = Amount(123);

			// Act:
			test::ObserveNotification(observer, notification, context);

			// Assert:
			test::AssertBalances(context.cache(), accountStateIter.get().PublicKey, { { Currency_Mosaic_Id, Amount(987) } });

			// - if harvester is remote, it should have an unchanged balance
			if (harvester != accountStateIter.get().PublicKey)
				test::AssertBalances(context.cache(), harvester, {});

			// - check (lack of) receipt
			auto pStatement = context.statementBuilder().build();
			ASSERT_EQ(0u, pStatement->TransactionStatements.size());
		});
	}

	// endregion

	// region fee sharing

	namespace {
		struct BalancesInfo {
			Amount HarvesterBalance;
			Amount BeneficiaryBalance;
			Amount NetworkBalance;
		};

		struct ReceiptInfo {
			Key Account;
			catapult::Amount Amount;
		};

		void AssertInflationReceipt(Amount expectedAmount, const model::InflationReceipt& receipt, const std::string& message) {
			ASSERT_EQ(sizeof(model::InflationReceipt), receipt.Size) << message;
			EXPECT_EQ(1u, receipt.Version) << message;
			EXPECT_EQ(model::Receipt_Type_Inflation, receipt.Type) << message;
			EXPECT_EQ(Currency_Mosaic_Id, receipt.Mosaic.MosaicId) << message;
			EXPECT_EQ(expectedAmount, receipt.Mosaic.Amount) << message;
		}

		struct HarvestFeeOptionsEx : public HarvestFeeOptions {
			Key HarvestNetworkFeeSinkPublicKey;
		};

		void AssertHarvesterSharesFees(
				NotifyMode notifyMode,
				const Key& harvester,
				const Key& beneficiary,
				const HarvestFeeOptionsEx& options,
				Amount totalFee,
				const model::InflationCalculator& calculator,
				const BalancesInfo& expectedFinalBalances,
				const std::vector<ReceiptInfo>& expectedReceiptInfos) {
			// Arrange:
			RunHarvestFeeObserverTest(notifyMode, options, calculator, [&](auto& context, const auto& observer) {
				// - setup cache
				auto& accountStateCache = context.cache().template sub<cache::AccountStateCache>();
				auto harvesterAccountStateIter = MainAccountTraits::AddAccount(accountStateCache, harvester, Height(1));
				harvesterAccountStateIter.get().Balances.credit(Currency_Mosaic_Id, Amount(987));

				auto beneficiaryAccountStateIter = MainAccountTraits::AddAccount(accountStateCache, beneficiary, Height(1));
				beneficiaryAccountStateIter.get().Balances.credit(Currency_Mosaic_Id, Amount(234));

				auto networkAccountStateIter = MainAccountTraits::AddAccount(
						accountStateCache,
						options.HarvestNetworkFeeSinkPublicKey,
						Height(1));
				networkAccountStateIter.get().Balances.credit(Currency_Mosaic_Id, Amount(444));

				auto notification = test::CreateBlockNotification(ToAddress(harvester), ToAddress(beneficiary));
				notification.TotalFee = totalFee;

				// Act:
				test::ObserveNotification(observer, notification, context);

				// Assert: balances
				test::AssertBalances(context.cache(), harvesterAccountStateIter.get().PublicKey, {
					{ Currency_Mosaic_Id, expectedFinalBalances.HarvesterBalance }
				});
				test::AssertBalances(context.cache(), beneficiaryAccountStateIter.get().PublicKey, {
					{ Currency_Mosaic_Id, expectedFinalBalances.BeneficiaryBalance }
				});
				test::AssertBalances(context.cache(), networkAccountStateIter.get().PublicKey, {
					{ Currency_Mosaic_Id, expectedFinalBalances.NetworkBalance }
				});

				// - check receipt(s)
				auto pStatement = context.statementBuilder().build();
				if (NotifyMode::Rollback == notifyMode) {
					EXPECT_TRUE(pStatement->TransactionStatements.empty());
					return;
				}

				ASSERT_EQ(1u, pStatement->TransactionStatements.size());
				const auto& receiptPair = *pStatement->TransactionStatements.find(model::ReceiptSource());
				ASSERT_EQ(expectedReceiptInfos.size(), receiptPair.second.size());

				auto inflationAmount = calculator.getSpotAmount(Observer_Context_Height);
				auto numInflationReceipts = Amount() == inflationAmount || NotifyMode::Rollback == notifyMode ? 0u : 1u;
				auto numReceipts = expectedReceiptInfos.size();
				for (auto i = 0u; i < numReceipts - numInflationReceipts; ++i) {
					const auto& receipt = static_cast<const model::BalanceChangeReceipt&>(receiptPair.second.receiptAt(i));
					const auto& expectedReceiptInfo = expectedReceiptInfos[i];
					auto message = "at index " + std::to_string(i);
					AssertReceipt(expectedReceiptInfo.Account, expectedReceiptInfo.Amount, receipt, message);
				}

				if (0 != numInflationReceipts) {
					const auto& receipt = static_cast<const model::InflationReceipt&>(receiptPair.second.receiptAt(numReceipts - 1));
					const auto& expectedReceiptInfo = expectedReceiptInfos[numReceipts - 1];
					AssertInflationReceipt(expectedReceiptInfo.Amount, receipt, "inflation receipt");
				}
			});
		}

		void AssertHarvesterSharesFees(
				const Key& harvester,
				const Key& beneficiary,
				const HarvestFeeOptionsEx& options,
				Amount totalFee,
				const BalancesInfo& expectedFinalBalances,
				const std::vector<ReceiptInfo>& expectedReceiptInfos) {
			AssertHarvesterSharesFees(
					NotifyMode::Commit,
					harvester,
					beneficiary,
					options,
					totalFee,
					model::InflationCalculator(),
					expectedFinalBalances,
					expectedReceiptInfos);
		}

		HarvestFeeOptionsEx CreateOptionsFromPercentages(uint8_t harvestBeneficiaryPercentage, uint8_t harvestNetworkPercentage) {
			// extend HarvestFeeOptions to add HarvestNetworkFeeSinkPublicKey so all accounts are represented as public keys
			auto options = HarvestFeeOptionsEx();
			options.HarvestNetworkFeeSinkPublicKey = test::GenerateRandomByteArray<Key>();
			options.CurrencyMosaicId = Currency_Mosaic_Id;
			options.HarvestBeneficiaryPercentage = harvestBeneficiaryPercentage;
			options.HarvestNetworkPercentage = harvestNetworkPercentage;
			options.HarvestNetworkFeeSinkAddress = ToAddress(options.HarvestNetworkFeeSinkPublicKey);
			return options;
		}
	}

	TEST(TEST_CLASS, HarvesterDoesNotShareFeesWhenPercentagesAreZero) {
		// Arrange:
		auto options = CreateOptionsFromPercentages(0, 0);
		auto harvester = test::GenerateRandomByteArray<Key>();
		auto beneficiary = test::GenerateRandomByteArray<Key>();
		BalancesInfo finalBalances{ Amount(987 + 205), Amount(234), Amount(444) };

		// Act + Assert:
		AssertHarvesterSharesFees(harvester, beneficiary, options, Amount(205), finalBalances, { { harvester, Amount(205) } });
	}

	TEST(TEST_CLASS, HarvesterDoesNotShareFeesWhenBeneficiaryIsEqualToHarvester) {
		// Arrange: harvester account (= beneficiary account) is initially credited 987 + 234
		auto options = CreateOptionsFromPercentages(20, 0);
		auto harvester = test::GenerateRandomByteArray<Key>();
		BalancesInfo finalBalances{ Amount(987 + 234 + 205), Amount(987 + 234 + 205), Amount(444) };

		// Act + Assert:
		AssertHarvesterSharesFees(harvester, harvester, options, Amount(205), finalBalances, { { harvester, Amount(205) } });
	}

	TEST(TEST_CLASS, HarvesterSharesFeesAccordingToGivenPercentage_BeneficiaryOnly_NoTruncation) {
		// Arrange: 205 * 0.2 = 41
		auto options = CreateOptionsFromPercentages(20, 0);
		auto harvester = test::GenerateRandomByteArray<Key>();
		auto beneficiary = test::GenerateRandomByteArray<Key>();
		BalancesInfo finalBalances{ Amount(987 + 164), Amount(234 + 41), Amount(444) };

		// Act + Assert:
		AssertHarvesterSharesFees(harvester, beneficiary, options, Amount(205), finalBalances, {
			{ harvester, Amount(164) }, { beneficiary, Amount(41) }
		});
	}

	TEST(TEST_CLASS, HarvesterSharesFeesAccordingToGivenPercentage_BeneficiaryOnly_Truncation) {
		// Arrange: 205 * 0.3 = 61.5
		auto options = CreateOptionsFromPercentages(30, 0);
		auto harvester = test::GenerateRandomByteArray<Key>();
		auto beneficiary = test::GenerateRandomByteArray<Key>();
		BalancesInfo finalBalances{ Amount(987 + 144), Amount(234 + 61), Amount(444) };

		// Act + Assert:
		AssertHarvesterSharesFees(harvester, beneficiary, options, Amount(205), finalBalances, {
			{ harvester, Amount(144) }, { beneficiary, Amount(61) }
		});
	}

	TEST(TEST_CLASS, HarvesterSharesFeesAccordingToGivenPercentage_NetworkOnly_NoTruncation) {
		// Arrange: 205 * 0.2 = 41
		auto options = CreateOptionsFromPercentages(0, 20);
		auto harvester = test::GenerateRandomByteArray<Key>();
		auto beneficiary = test::GenerateRandomByteArray<Key>();
		BalancesInfo finalBalances{ Amount(987 + 164), Amount(234), Amount(444 + 41) };

		// Act + Assert:
		AssertHarvesterSharesFees(harvester, beneficiary, options, Amount(205), finalBalances, {
			{ harvester, Amount(164) }, { options.HarvestNetworkFeeSinkPublicKey, Amount(41) }
		});
	}

	TEST(TEST_CLASS, HarvesterSharesFeesAccordingToGivenPercentage_NetworkOnly_Truncation) {
		// Arrange: 205 * 0.3 = 61.5
		auto options = CreateOptionsFromPercentages(0, 30);
		auto harvester = test::GenerateRandomByteArray<Key>();
		auto beneficiary = test::GenerateRandomByteArray<Key>();
		BalancesInfo finalBalances{ Amount(987 + 144), Amount(234), Amount(444 + 61) };

		// Act + Assert:
		AssertHarvesterSharesFees(harvester, beneficiary, options, Amount(205), finalBalances, {
			{ harvester, Amount(144) }, { options.HarvestNetworkFeeSinkPublicKey, Amount(61) }
		});
	}

	TEST(TEST_CLASS, HarvesterSharesFeesAccordingToGivenPercentage_BeneficiaryAndNetwork_NoTruncation) {
		// Arrange: 200 * 0.1 = 20, 200 * 0.2 = 40
		auto options = CreateOptionsFromPercentages(10, 20);
		auto harvester = test::GenerateRandomByteArray<Key>();
		auto beneficiary = test::GenerateRandomByteArray<Key>();
		BalancesInfo finalBalances{ Amount(987 + 140), Amount(234 + 20), Amount(444 + 40) };

		// Act + Assert:
		AssertHarvesterSharesFees(harvester, beneficiary, options, Amount(200), finalBalances, {
			{ harvester, Amount(140) }, { options.HarvestNetworkFeeSinkPublicKey, Amount(40) }, { beneficiary, Amount(20) }
		});
	}

	TEST(TEST_CLASS, HarvesterSharesFeesAccordingToGivenPercentage_BeneficiaryAndNetwork_Truncation) {
		// Arrange: 205 * 0.1 = 20.5, 205 * 0.3 = 61.5
		auto options = CreateOptionsFromPercentages(10, 30);
		auto harvester = test::GenerateRandomByteArray<Key>();
		auto beneficiary = test::GenerateRandomByteArray<Key>();
		BalancesInfo finalBalances{ Amount(987 + 124), Amount(234 + 20), Amount(444 + 61) };

		// Act + Assert:
		AssertHarvesterSharesFees(harvester, beneficiary, options, Amount(205), finalBalances, {
			{ harvester, Amount(124) }, { options.HarvestNetworkFeeSinkPublicKey, Amount(61) }, { beneficiary, Amount(20) }
		});
	}

	TEST(TEST_CLASS, NoAdditionalReceiptIsGeneratedWhenTruncatedAmountIsZero) {
		// Arrange:
		auto options = CreateOptionsFromPercentages(30, 30);
		auto harvester = test::GenerateRandomByteArray<Key>();
		auto beneficiary = test::GenerateRandomByteArray<Key>();
		BalancesInfo finalBalances{ Amount(987 + 1), Amount(234), Amount(444) };

		// Act + Assert:
		AssertHarvesterSharesFees(harvester, beneficiary, options, Amount(1), finalBalances, { { harvester, Amount(1) } });
	}

	// endregion

	// region fee sharing - remote benficiary

	namespace {
		template<typename TTraits, typename TAssert>
		void AssertHarvesterSharesFees(NotifyMode mode, TAssert assertBalancesAndReceipts) {
			RunHarvestFeeObserverTest(mode, 20, [&](auto& context, const auto& observer) {
				// Arrange: setup cache
				auto harvester = test::GenerateRandomByteArray<Key>();
				auto beneficiary = test::GenerateRandomByteArray<Key>();
				auto& accountStateCache = context.cache().template sub<cache::AccountStateCache>();
				auto harvesterAccountStateIter = TTraits::AddAccount(accountStateCache, harvester, Height(1));
				harvesterAccountStateIter.get().Balances.credit(Currency_Mosaic_Id, Amount(987));
				auto beneficiaryAccountStateIter = TTraits::AddAccount(accountStateCache, beneficiary, Height(1));
				beneficiaryAccountStateIter.get().Balances.credit(Currency_Mosaic_Id, Amount(234));

				auto notification = test::CreateBlockNotification(ToAddress(harvester), ToAddress(beneficiary));
				notification.TotalFee = Amount(205);

				// Act:
				test::ObserveNotification(observer, notification, context);

				// Assert: if beneficiary is remote, it should have an unchanged balance
				const auto& mainHarvesterPublicKey = harvesterAccountStateIter.get().PublicKey;
				const auto& mainBeneficiaryPublicKey = beneficiaryAccountStateIter.get().PublicKey;
				if (beneficiary != mainBeneficiaryPublicKey)
					test::AssertBalances(context.cache(), beneficiary, {});

				assertBalancesAndReceipts(context, mainHarvesterPublicKey, mainBeneficiaryPublicKey);
			});
		}
	}

	ACCOUNT_TYPE_TRAITS_BASED_TEST(HarvesterSharesFeesWithMainAccountOfRemoteBeneficiary_Commit) {
		// Arrange:
		AssertHarvesterSharesFees<TTraits>(NotifyMode::Commit, [](auto& context, const auto& mainHarvester, const auto& mainBeneficiary) {
			// Assert: harvester balance: 987 + 164
			test::AssertBalances(context.cache(), mainHarvester, { { Currency_Mosaic_Id, Amount(1151) } });

			// - main beneficiary balance: 234 + 41
			test::AssertBalances(context.cache(), mainBeneficiary, { { Currency_Mosaic_Id, Amount(275) } });

			// - check receipt(s)
			auto pStatement = context.statementBuilder().build();
			ASSERT_EQ(1u, pStatement->TransactionStatements.size());
			const auto& receiptPair = *pStatement->TransactionStatements.find(model::ReceiptSource());
			ASSERT_EQ(2u, receiptPair.second.size());

			// - harvester receipt
			const auto& harvesterReceipt = static_cast<const model::BalanceChangeReceipt&>(receiptPair.second.receiptAt(0));
			AssertReceipt(mainHarvester, Amount(164), harvesterReceipt);

			// - main beneficiary receipt
			const auto& mainBeneficiaryReceipt = static_cast<const model::BalanceChangeReceipt&>(receiptPair.second.receiptAt(1));
			AssertReceipt(mainBeneficiary, Amount(41), mainBeneficiaryReceipt);
		});
	}

	ACCOUNT_TYPE_TRAITS_BASED_TEST(HarvesterSharesFeesWithMainAccountOfRemoteBeneficiary_Rollback) {
		// Arrange:
		auto notifyMode = NotifyMode::Rollback;
		AssertHarvesterSharesFees<TTraits>(notifyMode, [](auto& context, const auto& mainHarvester, const auto& mainBeneficiary) {
			// Assert: harvester balance: 987 - 164
			test::AssertBalances(context.cache(), mainHarvester, { { Currency_Mosaic_Id, Amount(823) } });

			// - main beneficiary balance: 234 - 41
			test::AssertBalances(context.cache(), mainBeneficiary, { { Currency_Mosaic_Id, Amount(193) } });

			// - no receipt
			auto pStatement = context.statementBuilder().build();
			ASSERT_EQ(0u, pStatement->TransactionStatements.size());
		});
	}

	// endregion

	// region inflation

	namespace {
		auto CreateCustomCalculator() {
			auto calculator = model::InflationCalculator();
			calculator.add(Height(123), Amount(100));
			calculator.add(Height(555), Amount(200));
			calculator.add(Height(987), Amount(300));
			return calculator;
		}
	}

	TEST(TEST_CLASS, HarvesterSharesInflationAccordingToGivenPercentage_Commit) {
		// Arrange: (500 + 200) * 0.2 = 140, (500 + 200) * 0.1 = 70,
		//          initial balances are 987 for harvester, 234 for beneficiary, 444 for network
		//          context height is 555
		auto options = CreateOptionsFromPercentages(20, 10);
		auto harvester = test::GenerateRandomByteArray<Key>();
		auto beneficiary = test::GenerateRandomByteArray<Key>();
		auto calculator = CreateCustomCalculator();
		BalancesInfo finalBalances{ Amount(987 + 490), Amount(234 + 140), Amount(444 + 70) };

		// Act + Assert: last receipt is the expected inflation receipt
		AssertHarvesterSharesFees(NotifyMode::Commit, harvester, beneficiary, options, Amount(500), calculator, finalBalances, {
			{ harvester, Amount(490) },
			{ options.HarvestNetworkFeeSinkPublicKey, Amount(70) },
			{ beneficiary, Amount(140) },
			{ Key(), Amount(200) }
		});
	}

	TEST(TEST_CLASS, HarvesterSharesInflationAccordingToGivenPercentage_Rollback) {
		// Arrange: (500 + 200) * 0.2 = 140, (500 + 200) * 0.1 = 70,
		//          initial balances are 987 for harvester, 234 for beneficiary, 444 for network
		//          context height is 555
		auto options = CreateOptionsFromPercentages(20, 10);
		auto harvester = test::GenerateRandomByteArray<Key>();
		auto beneficiary = test::GenerateRandomByteArray<Key>();
		auto calculator = CreateCustomCalculator();
		BalancesInfo finalBalances{ Amount(987 - 490), Amount(234 - 140), Amount(444 - 70) };

		// Act + Assert:
		AssertHarvesterSharesFees(NotifyMode::Rollback, harvester, beneficiary, options, Amount(500), calculator, finalBalances, {});
	}

	// endregion

	// region improper link

	namespace {
		template<typename TMutator>
		void AssertImproperLink(TMutator mutator) {
			// Arrange:
			test::AccountObserverTestContext context(NotifyMode::Commit);
			auto& accountStateCache = context.cache().sub<cache::AccountStateCache>();
			auto pObserver = CreateHarvestFeeObserver(CreateOptionsFromPercentages(20, 0), model::InflationCalculator());

			auto harvester = test::GenerateRandomByteArray<Key>();
			auto accountStateIter = RemoteAccountTraits::AddAccount(accountStateCache, harvester, Height(1));
			accountStateIter.get().Balances.credit(Currency_Mosaic_Id, Amount(987));
			mutator(accountStateIter.get());

			// - notification.Beneficiary must be valid because percentage is nonzero
			auto beneficiary = test::GenerateRandomByteArray<Key>();
			auto notification = test::CreateBlockNotification(ToAddress(harvester), ToAddress(beneficiary));
			notification.TotalFee = Amount(123);

			// Act + Assert:
			EXPECT_THROW(test::ObserveNotification(*pObserver, notification, context), catapult_runtime_error);
		}
	}

	TEST(TEST_CLASS, FailureWhenLinkedAccountHasWrongType) {
		AssertImproperLink([](auto& accountState) {
			// Arrange: change the main account to have the wrong type
			accountState.AccountType = state::AccountType::Remote;
		});
	}

	TEST(TEST_CLASS, FailureWhenLinkedAccountDoesNotReferenceRemoteAccount) {
		AssertImproperLink([](auto& accountState) {
			// Arrange: change the main account to point to a different account
			test::ForceSetLinkedPublicKey(accountState, test::GenerateRandomByteArray<Key>());
		});
	}

	// endregion
}}

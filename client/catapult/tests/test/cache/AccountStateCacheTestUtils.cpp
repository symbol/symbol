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

#include "AccountStateCacheTestUtils.h"
#include "catapult/cache_core/AccountStateCacheDelta.h"
#include "tests/TestHarness.h"

namespace catapult {
namespace test {

    state::AccountHistory CreateAccountHistory(const std::vector<std::pair<Height, Amount>>& balancePairs)
    {
        state::AccountHistory history;
        for (const auto& pair : balancePairs)
            history.add(pair.first, pair.second);

        // Sanity: all pairs were added
        EXPECT_EQ(balancePairs.size(), history.balance().size());
        return history;
    }

    cache::AddressAccountHistoryMap GenerateAccountHistories(const AddressBalanceHistorySeeds& seeds)
    {
        cache::AddressAccountHistoryMap map;

        for (const auto& seed : seeds)
            map.emplace(Address { { seed.first } }, CreateAccountHistory(seed.second));

        return map;
    }

    std::vector<Address> AddAccountsWithBalances(
        cache::AccountStateCacheDelta& delta,
        MosaicId mosaicId,
        const std::vector<Amount>& balances)
    {
        auto addresses = test::GenerateRandomDataVector<Address>(balances.size());
        for (auto i = 0u; i < balances.size(); ++i) {
            delta.addAccount(addresses[i], Height(1));
            auto& accountState = delta.find(addresses[i]).get();
            accountState.SupplementalPublicKeys.voting().add(test::GenerateRandomPackedStruct<model::PinnedVotingKey>());
            accountState.Balances.credit(mosaicId, balances[i]);
        }

        return addresses;
    }

    std::vector<Address> AddAccountsWithBalancesAndOverlappingVotingKeyLifetimes(
        cache::AccountStateCacheDelta& delta,
        MosaicId mosaicId,
        const std::vector<Amount>& balances)
    {
        auto addresses = AddAccountsWithBalances(delta, mosaicId, balances);

        auto i = 1u;
        for (const auto& address : addresses) {
            auto accountIter = delta.find(address);
            auto& accountState = accountIter.get();

            // remove existing voting key and add one with 50 epoch lifetime
            auto& votingPublicKeys = accountState.SupplementalPublicKeys.voting();
            votingPublicKeys.remove(votingPublicKeys.get(0));
            votingPublicKeys.add({ GenerateRandomByteArray<VotingKey>(), FinalizationEpoch(i * 10), FinalizationEpoch((i + 5) * 10) });
            ++i;
        }

        return addresses;
    }

    namespace {
        enum class AccountHistoryEqualityPolicy { All,
            Balance_Only };

        template <typename TValue>
        void AssertEqual(
            const state::HeightIndexedHistoryMap<TValue>& expected,
            const state::HeightIndexedHistoryMap<TValue>& actual,
            const std::string& message)
        {
            EXPECT_EQ(expected.heights(), actual.heights()) << message;

            for (auto height : expected.heights())
                EXPECT_EQ(expected.get(height), actual.get(height)) << message << ", height = " << height;
        }

        void AssertEqual(
            AccountHistoryEqualityPolicy equalityPolicy,
            const cache::AddressAccountHistoryMap& expected,
            const cache::AddressAccountHistoryMap& actual)
        {
            EXPECT_EQ(expected.size(), actual.size());

            for (const auto& pair : expected) {
                auto actualIter = actual.find(pair.first);
                if (actual.cend() == actualIter) {
                    EXPECT_NE(actual.cend(), actualIter) << pair.first << " in expected but not in actual";
                    continue;
                }

                std::ostringstream messagePostfixStream;
                messagePostfixStream << " address = " << pair.first;
                auto messagePostfix = messagePostfixStream.str();

                AssertEqual(pair.second.balance(), actualIter->second.balance(), "balance" + messagePostfix);
                if (AccountHistoryEqualityPolicy::Balance_Only == equalityPolicy)
                    continue;

                AssertEqual(pair.second.vrfPublicKey(), actualIter->second.vrfPublicKey(), "vrfPublicKey" + messagePostfix);
                AssertEqual(pair.second.votingPublicKeys(), actualIter->second.votingPublicKeys(), "votingPublicKeys" + messagePostfix);
            }
        }
    }

    void AssertEqual(const cache::AddressAccountHistoryMap& expected, const cache::AddressAccountHistoryMap& actual)
    {
        AssertEqual(AccountHistoryEqualityPolicy::All, expected, actual);
    }

    void AssertEqualBalanceHistoryOnly(const cache::AddressAccountHistoryMap& expected, const cache::AddressAccountHistoryMap& actual)
    {
        AssertEqual(AccountHistoryEqualityPolicy::Balance_Only, expected, actual);
    }
}
}

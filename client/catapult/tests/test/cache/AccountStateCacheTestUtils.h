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

#pragma once
#include "catapult/cache_core/AccountStateCacheTypes.h"
#include "catapult/cache_core/HighValueAccounts.h"

namespace catapult { namespace test {

	/// Creates account state cache options for use in tests with \a currencyMosaicId and \a harvestingMosaicId.
	constexpr cache::AccountStateCacheTypes::Options CreateDefaultAccountStateCacheOptions(
			MosaicId currencyMosaicId,
			MosaicId harvestingMosaicId) {
		return {
			model::NetworkIdentifier::Private_Test,
			333,
			222,
			Amount(),
			Amount(std::numeric_limits<Amount::ValueType>::max()),
			Amount(),
			currencyMosaicId,
			harvestingMosaicId
		};
	}

	/// Creates default account state cache options for use in tests.
	constexpr cache::AccountStateCacheTypes::Options CreateDefaultAccountStateCacheOptions() {
		return CreateDefaultAccountStateCacheOptions(MosaicId(1111), MosaicId(2222));
	}

	/// Creates an account history given the specified height and balance pairs (\a balancePairs).
	state::AccountHistory CreateAccountHistory(const std::vector<std::pair<Height, Amount>>& balancePairs);

	/// Balance seed data for generating an address account history map.
	using AddressBalanceHistorySeeds = std::vector<std::pair<Address, std::vector<std::pair<Height, Amount>>>>;

	/// Generates an address account history map from balance \a seeds.
	cache::AddressAccountHistoryMap GenerateAccountHistories(const AddressBalanceHistorySeeds& seeds);

	/// Asserts that \a expected and \a actual are equal.
	void AssertEqual(const cache::AddressAccountHistoryMap& expected, const cache::AddressAccountHistoryMap& actual);

	/// Asserts that \a expected and \a actual have equal balance histories only.
	void AssertEqualBalanceHistoryOnly(const cache::AddressAccountHistoryMap& expected, const cache::AddressAccountHistoryMap& actual);
}}

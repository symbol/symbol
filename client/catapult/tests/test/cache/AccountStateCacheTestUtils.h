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
			model::NetworkIdentifier::Mijin_Test,
			543,
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

	/// Creates a balance history given the specified height and balance pairs (\a balancePairs).
	state::BalanceHistory CreateBalanceHistory(const std::vector<std::pair<Height, Amount>>& balancePairs);

	/// Seed data for generating an address balance history map.
	using AddressBalanceHistorySeeds = std::vector<std::pair<Address, std::vector<std::pair<Height, Amount>>>>;

	/// Generates an address balance history map from \a seeds.
	cache::AddressBalanceHistoryMap GenerateBalanceHistories(const AddressBalanceHistorySeeds& seeds);

	/// Asserts that \a expected and \a actual are equal.
	void AssertEqual(const cache::AddressBalanceHistoryMap& expected, const cache::AddressBalanceHistoryMap& actual);
}}

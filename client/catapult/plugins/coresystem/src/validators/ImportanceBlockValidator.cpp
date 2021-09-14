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

#include "Validators.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/VotingSet.h"

namespace catapult { namespace validators {

	using Notification = model::ImportanceBlockNotification;

	namespace {
		bool AreEqual(const Notification& notification, const cache::HighValueAccountStatistics& statistics) {
			return notification.VotingEligibleAccountsCount == statistics.VotingEligibleAccountsCount
					&& notification.HarvestingEligibleAccountsCount == statistics.HarvestingEligibleAccountsCount
					&& notification.TotalVotingBalance == statistics.TotalVotingBalance;
		}
	}

	DECLARE_STATEFUL_VALIDATOR(ImportanceBlock, Notification)(
			Height totalVotingBalanceCalculationFixForkHeight,
			uint64_t votingSetGrouping) {
		return MAKE_STATEFUL_VALIDATOR(ImportanceBlock, ([totalVotingBalanceCalculationFixForkHeight, votingSetGrouping](
				const Notification& notification,
				const ValidatorContext& context) {
			auto epoch = context.Height < totalVotingBalanceCalculationFixForkHeight
					? FinalizationEpoch(0)
					: model::CalculateFinalizationEpochForHeight(context.Height, votingSetGrouping);

			auto statistics = context.Cache.sub<cache::AccountStateCache>().highValueAccountStatistics(epoch);
			if (AreEqual(notification, statistics))
				return ValidationResult::Success;

			CATAPULT_LOG(debug)
					<< "detected importance block mismatch at " << context.Height
					<< std::endl << "VotingEligibleAccountsCount"
					<< std::endl << " + notification " << notification.VotingEligibleAccountsCount
					<< std::endl << " + statistics   " << statistics.VotingEligibleAccountsCount
					<< std::endl << "HarvestingEligibleAccountsCount"
					<< std::endl << " + notification " << notification.HarvestingEligibleAccountsCount
					<< std::endl << " + statistics   " << statistics.HarvestingEligibleAccountsCount
					<< std::endl << "TotalVotingBalance"
					<< std::endl << " + notification " << notification.TotalVotingBalance
					<< std::endl << " + statistics   " << statistics.TotalVotingBalance;

			return Failure_Core_Importance_Block_Mismatch;
		}));
	}
}}

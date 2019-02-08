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

#include "PredicateUtils.h"
#include "TransactionSpamThrottle.h"
#include "catapult/cache/UtCache.h"
#include "catapult/config/LocalNodeConfiguration.h"

namespace catapult { namespace sync {

	model::MatchingEntityPredicate ToRequiresValidationPredicate(const chain::KnownHashPredicate& knownHashPredicate) {
		return [knownHashPredicate](auto entityType, auto timestamp, const auto& hash) {
			auto isTransaction = model::BasicEntityType::Transaction == entityType;
			return !isTransaction || !knownHashPredicate(timestamp, hash);
		};
	}

	namespace {
		bool IsBondedTransaction(const model::Transaction& transaction) {
			return transaction.Type == model::MakeEntityType(model::BasicEntityType::Transaction, model::FacilityCode::Aggregate, 2);
		}

		chain::UtUpdater::Throttle CreateDefaultUtUpdaterThrottle(uint64_t maxCacheSize) {
			return [maxCacheSize](const auto&, const auto& context) {
				return context.TransactionsCache.size() >= maxCacheSize;
			};
		}
	}

	chain::UtUpdater::Throttle CreateUtUpdaterThrottle(const config::LocalNodeConfiguration& config) {
		SpamThrottleConfiguration throttleConfig(
				config.Node.TransactionSpamThrottlingMaxBoostFee,
				config.BlockChain.TotalChainImportance,
				config.Node.UnconfirmedTransactionsCacheMaxSize,
				config.BlockChain.MaxTransactionsPerBlock);

		return config.Node.ShouldEnableTransactionSpamThrottling
				? CreateTransactionSpamThrottle(throttleConfig, IsBondedTransaction)
				: CreateDefaultUtUpdaterThrottle(throttleConfig.MaxCacheSize);
	}
}}

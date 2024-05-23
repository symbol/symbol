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

#pragma once
#include "catapult/cache_core/AccountStateCache.h"

namespace catapult {
namespace observers {

    /// On commit, credits the expiration account(s) of expired locks and creates receipts of \a receiptType.
    /// On rollback, debits the expiration account(s) of expired locks.
    /// Uses the observer \a context to determine notification direction and access caches.
    /// Uses \a ownerAccountStateDispatcher to retrieve the lock owner's account state.
    template <typename TLockInfoCache, typename TAccountStateDispatcher>
    void ExpiredLockInfoObserver(
        ObserverContext& context,
        model::ReceiptType receiptType,
        TAccountStateDispatcher ownerAccountStateDispatcher)
    {
        std::vector<std::unique_ptr<model::BalanceChangeReceipt>> receipts;
        auto receiptAppender = [&receipts, receiptType](const auto& address, auto mosaicId, auto amount) {
            receipts.push_back(std::make_unique<model::BalanceChangeReceipt>(receiptType, address, mosaicId, amount));
        };

        auto& lockInfoCache = context.Cache.template sub<TLockInfoCache>();
        lockInfoCache.processUnusedExpiredLocks(
            context.Height,
            [&context, ownerAccountStateDispatcher, receiptAppender](const auto& lockInfo) {
                auto& accountStateCache = context.Cache.sub<cache::AccountStateCache>();
                ownerAccountStateDispatcher(accountStateCache, lockInfo, [&context, &lockInfo, receiptAppender](auto& accountState) {
                    if (NotifyMode::Rollback == context.Mode) {
                        accountState.Balances.debit(lockInfo.MosaicId, lockInfo.Amount);
                        return;
                    }

                    accountState.Balances.credit(lockInfo.MosaicId, lockInfo.Amount);
                    receiptAppender(accountState.Address, lockInfo.MosaicId, lockInfo.Amount);
                });
            });

        // sort receipts in order to fulfill deterministic ordering requirement
        std::sort(receipts.begin(), receipts.end(), [](const auto& pLhs, const auto& pRhs) {
            return std::memcmp(pLhs.get(), pRhs.get(), sizeof(model::BalanceChangeReceipt)) < 0;
        });

        for (const auto& pReceipt : receipts)
            context.StatementBuilder().addReceipt(*pReceipt);
    }
}
}

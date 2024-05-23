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
#include "catapult/functions.h"
#include "catapult/model/EntityInfo.h"
#include "catapult/types.h"

namespace catapult {
namespace harvesting {

    /// Information about a fee policy.
    struct FeePolicy {
        /// Number of transactions.
        uint32_t NumTransactions = 0;

        /// Fee multiplier.
        BlockFeeMultiplier FeeMultiplier;

        /// Base fee.
        Amount BaseFee;
    };

    /// Maximizes fees given a stream of transaction infos.
    class TransactionFeeMaximizer {
    public:
        /// Gets the best fee policy identified.
        const FeePolicy& best() const;

    public:
        /// Applies \a transactionInfo to the maximizer to include in the best fee policy calculation.
        void apply(const model::TransactionInfo& transactionInfo);

    private:
        FeePolicy m_current;
        FeePolicy m_best;
    };
}
}

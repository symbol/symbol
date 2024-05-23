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
#include "TransactionBuilder.h"
#include "plugins/txes/restriction_account/src/model/AccountOperationRestrictionTransaction.h"
#include <vector>

namespace catapult {
namespace builders {

    /// Builder for an account operation restriction transaction.
    class AccountOperationRestrictionBuilder : public TransactionBuilder {
    public:
        using Transaction = model::AccountOperationRestrictionTransaction;
        using EmbeddedTransaction = model::EmbeddedAccountOperationRestrictionTransaction;

    public:
        /// Creates an account operation restriction builder for building an account operation restriction transaction from \a signer
        /// for the network specified by \a networkIdentifier.
        AccountOperationRestrictionBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer);

    public:
        /// Sets the account restriction flags to \a restrictionFlags.
        void setRestrictionFlags(model::AccountRestrictionFlags restrictionFlags);

        /// Adds \a restrictionAddition to account restriction additions.
        void addRestrictionAddition(model::EntityType restrictionAddition);

        /// Adds \a restrictionDeletion to account restriction deletions.
        void addRestrictionDeletion(model::EntityType restrictionDeletion);

    public:
        /// Gets the size of account operation restriction transaction.
        /// \note This returns size of a normal transaction not embedded transaction.
        size_t size() const;

        /// Builds a new account operation restriction transaction.
        std::unique_ptr<Transaction> build() const;

        /// Builds a new embedded account operation restriction transaction.
        std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

    private:
        template <typename TTransaction>
        size_t sizeImpl() const;

        template <typename TTransaction>
        std::unique_ptr<TTransaction> buildImpl() const;

    private:
        model::AccountRestrictionFlags m_restrictionFlags;
        std::vector<model::EntityType> m_restrictionAdditions;
        std::vector<model::EntityType> m_restrictionDeletions;
    };
}
}

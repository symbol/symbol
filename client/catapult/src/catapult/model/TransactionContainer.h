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
#include "ContiguousEntityContainer.h"
#include "SizePrefixedEntity.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a transaction container.
	/// \note This container only assumes components of type 'SizePrefixedEntity' but is named 'Transaction' because all current
	///       usages are containers of some type of transactions and this naming leads to more natural usage
	///       (i.e. Transactions vs Entities).
	template<typename TEntityHeader, typename TComponentEntity>
	struct TransactionContainer : public TEntityHeader {
		/// Returns a container wrapping the transactions contained in this container with the
		/// desired error policy (\a errorPolicy).
		auto Transactions(EntityContainerErrorPolicy errorPolicy = EntityContainerErrorPolicy::Throw) {
			return MakeContiguousEntityContainer(TransactionsPtr(), GetTransactionPayloadSize(*this), errorPolicy);
		}

		/// Returns a container wrapping the const transactions contained in this container with the
		/// desired error policy (\a errorPolicy).
		auto Transactions(EntityContainerErrorPolicy errorPolicy = EntityContainerErrorPolicy::Throw) const {
			return MakeContiguousEntityContainer(TransactionsPtr(), GetTransactionPayloadSize(*this), errorPolicy);
		}

		/// Returns a pointer to transactions contained in this container.
		TComponentEntity* TransactionsPtr() {
			return TEntityHeader::Size <= sizeof(TEntityHeader) || 0 == GetTransactionPayloadSize(*this)
					? nullptr
					: reinterpret_cast<TComponentEntity*>(reinterpret_cast<uint8_t*>(this) + sizeof(TEntityHeader));
		}

		/// Returns a const pointer to transactions contained in this container.
		const TComponentEntity* TransactionsPtr() const {
			return TEntityHeader::Size <= sizeof(TEntityHeader) || 0 == GetTransactionPayloadSize(*this)
					? nullptr
					: reinterpret_cast<const TComponentEntity*>(reinterpret_cast<const uint8_t*>(this) + sizeof(TEntityHeader));
		}

		// followed by "transactions"
	};

#pragma pack(pop)
}}

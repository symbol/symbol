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
#include "LinkAction.h"
#include "Transaction.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a key link transaction body.
	template<typename THeader, typename TAccountPublicKey, EntityType Key_Link_Entity_Type>
	struct BasicKeyLinkTransactionBody : public THeader {
	private:
		using TransactionType = BasicKeyLinkTransactionBody<THeader, TAccountPublicKey, Key_Link_Entity_Type>;

	public:
		DEFINE_TRANSACTION_CONSTANTS(Key_Link_Entity_Type, 1)

	public:
		/// Linked public key.
		TAccountPublicKey LinkedPublicKey;

		/// Link action.
		model::LinkAction LinkAction;

	public:
		/// Calculates the real size of key link \a transaction.
		static constexpr uint64_t CalculateRealSize(const TransactionType&) noexcept {
			return sizeof(TransactionType);
		}
	};

#pragma pack(pop)
}}

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
#include "AccountRestrictionEntityType.h"
#include "AccountRestrictionTypes.h"
#include "catapult/model/Transaction.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a basic account restriction transaction body.
	template<typename THeader, typename TAccountRestrictionModification>
	struct BasicAccountRestrictionTransactionBody : public THeader {
	private:
		using TransactionType = BasicAccountRestrictionTransactionBody<THeader, TAccountRestrictionModification>;

	public:
		/// Account restriction type.
		AccountRestrictionType RestrictionType;

		/// Number of modifications.
		uint8_t ModificationsCount;

		// followed by account restriction modifications if ModificationsCount != 0
		DEFINE_TRANSACTION_VARIABLE_DATA_ACCESSORS(Modifications, TAccountRestrictionModification)

	private:
		template<typename T>
		static auto* ModificationsPtrT(T& transaction) {
			return transaction.ModificationsCount ? THeader::PayloadStart(transaction) : nullptr;
		}

	public:
		// Calculates the real size of account restriction \a transaction.
		static constexpr uint64_t CalculateRealSize(const TransactionType& transaction) noexcept {
			return sizeof(TransactionType) + transaction.ModificationsCount * sizeof(TAccountRestrictionModification);
		}
	};

#define DEFINE_ACCOUNT_RESTRICTION_TRANSACTION(VALUE_NAME, ENTITY_TYPE_NAME, VALUE_TYPE) \
	template<typename THeader> \
	struct Account##VALUE_NAME##RestrictionTransactionBody \
			: public BasicAccountRestrictionTransactionBody<THeader, AccountRestrictionModification<VALUE_TYPE>> { \
	public: \
		DEFINE_TRANSACTION_CONSTANTS(Entity_Type_Account_##ENTITY_TYPE_NAME##_Restriction, 1) \
	}; \
	\
	DEFINE_EMBEDDABLE_TRANSACTION(Account##VALUE_NAME##Restriction)

#pragma pack(pop)
}}

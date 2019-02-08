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
#include "PropertyEntityType.h"
#include "PropertyTypes.h"
#include "catapult/model/Transaction.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a basic property transaction body.
	template<typename THeader, typename TPropertyModification>
	struct BasicPropertyTransactionBody : public THeader {
	private:
		using TransactionType = BasicPropertyTransactionBody<THeader, TPropertyModification>;

	public:
		/// Property type.
		model::PropertyType PropertyType;

		/// Number of modifications.
		uint8_t ModificationsCount;

		// followed by property modifications if ModificationsCount != 0
		DEFINE_TRANSACTION_VARIABLE_DATA_ACCESSORS(Modifications, TPropertyModification)

	private:
		template<typename T>
		static auto* ModificationsPtrT(T& transaction) {
			return transaction.ModificationsCount ? THeader::PayloadStart(transaction) : nullptr;
		}

	public:
		// Calculates the real size of property \a transaction.
		static constexpr uint64_t CalculateRealSize(const TransactionType& transaction) noexcept {
			return sizeof(TransactionType) + transaction.ModificationsCount * sizeof(TPropertyModification);
		}
	};

#define DEFINE_PROPERTY_TRANSACTION(VALUE_NAME, ENTITY_TYPE_NAME, VALUE_TYPE) \
	template<typename THeader> \
	struct VALUE_NAME##PropertyTransactionBody : public BasicPropertyTransactionBody<THeader, PropertyModification<VALUE_TYPE>> { \
	public: \
		DEFINE_TRANSACTION_CONSTANTS(Entity_Type_##ENTITY_TYPE_NAME##_Property, 1) \
	}; \
	\
	DEFINE_EMBEDDABLE_TRANSACTION(VALUE_NAME##Property)

#pragma pack(pop)
}}

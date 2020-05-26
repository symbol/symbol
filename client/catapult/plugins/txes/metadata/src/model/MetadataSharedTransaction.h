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
#include "catapult/model/Transaction.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Metadata transaction header.
	template<typename THeader>
	struct MetadataTransactionHeader : public THeader {
		/// Metadata target address.
		UnresolvedAddress TargetAddress;

		/// Metadata key scoped to source, target and type.
		uint64_t ScopedMetadataKey;
	};

	/// Binary layout for a basic metadata transaction body.
	template<typename THeader, EntityType Metadata_Entity_Type>
	struct BasicMetadataTransactionBody : public THeader {
	private:
		using TransactionType = BasicMetadataTransactionBody<THeader, Metadata_Entity_Type>;

	public:
		DEFINE_TRANSACTION_CONSTANTS(Metadata_Entity_Type, 1)

	public:
		/// Change in value size in bytes.
		int16_t ValueSizeDelta;

		/// Value size in bytes.
		uint16_t ValueSize;

		// followed by value data if ValueSize != 0
		DEFINE_TRANSACTION_VARIABLE_DATA_ACCESSORS(Value, uint8_t)

	private:
		template<typename T>
		static auto* ValuePtrT(T& transaction) {
			return transaction.ValueSize ? THeader::PayloadStart(transaction) : nullptr;
		}

	public:
		/// Calculates the real size of metadata \a transaction.
		static constexpr uint64_t CalculateRealSize(const TransactionType& transaction) noexcept {
			return sizeof(TransactionType) + transaction.ValueSize;
		}
	};

#pragma pack(pop)
}}

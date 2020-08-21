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
#include "AggregateEntityType.h"
#include "catapult/model/Cosignature.h"
#include "catapult/model/EntityType.h"
#include "catapult/model/SizePrefixedEntityContainer.h"
#include "catapult/model/Transaction.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for an aggregate transaction header.
	struct AggregateTransactionHeader : public Transaction {
	public:
		DEFINE_TRANSACTION_CONSTANTS(Entity_Type_Aggregate_Complete, 1)

		/// Size of the footer that can be skipped when signing/verifying.
		static constexpr size_t Footer_Size = 2 * sizeof(uint32_t);

	public:
		/// Aggregate hash of an aggregate's transactions.
		Hash256 TransactionsHash;

		/// Transaction payload size in bytes.
		/// \note This is the total number bytes occupied by all sub-transactions.
		uint32_t PayloadSize;

		/// Reserved padding to align end of AggregateTransactionHeader on 8-byte boundary.
		uint32_t AggregateTransactionHeader_Reserved1;

		// followed by sub-transaction data
		// followed by cosignatures data
	};

	/// Binary layout for an aggregate transaction.
	struct AggregateTransaction : public TransactionContainer<AggregateTransactionHeader, EmbeddedTransaction> {
	private:
		template<typename T>
		static auto* CosignaturesPtrT(T& transaction) {
			return transaction.Size <= sizeof(T) + transaction.PayloadSize
					? nullptr
					: transaction.ToBytePointer() + sizeof(T) + transaction.PayloadSize;
		}

		template<typename T>
		static size_t CosignaturesCountT(T& transaction) {
			return transaction.Size <= sizeof(T) + transaction.PayloadSize
					? 0
					: (transaction.Size - sizeof(T) - transaction.PayloadSize) / sizeof(Cosignature);
		}

	public:
		DEFINE_SIZE_PREFIXED_ENTITY_VARIABLE_DATA_ACCESSORS(Cosignatures, Cosignature)

		/// Gets the number of cosignatures attached to this transaction.
		/// \note The returned value is undefined if the aggregate has an invalid size.
		size_t CosignaturesCount() const {
			return CosignaturesCountT(*this);
		}

		/// Gets the number of cosignatures attached to this transaction.
		/// \note The returned value is undefined if the aggregate has an invalid size.
		size_t CosignaturesCount() {
			return CosignaturesCountT(*this);
		}
	};

#pragma pack(pop)

	/// Gets the number of bytes containing transaction data according to \a header.
	size_t GetTransactionPayloadSize(const AggregateTransactionHeader& header);

	/// Checks the real size of \a aggregate against its reported size and returns \c true if the sizes match.
	/// \a registry contains all known transaction types.
	bool IsSizeValid(const AggregateTransaction& aggregate, const TransactionRegistry& registry);
}}

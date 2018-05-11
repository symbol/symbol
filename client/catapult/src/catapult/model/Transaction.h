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
#include "EmbeddedTransaction.h"
#include "VerifiableEntity.h"

namespace catapult { namespace model { class TransactionRegistry; } }

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a transaction.
	struct Transaction : public VerifiableEntity {
		/// Transaction fee.
		Amount Fee;

		/// Transaction deadline.
		Timestamp Deadline;
	};

#pragma pack(pop)

	/// Checks the real size of \a transaction against its reported size and returns \c true if the sizes match.
	/// \a registry contains all known transaction types.
	bool IsSizeValid(const Transaction& transaction, const TransactionRegistry& registry);

	// region macros

/// Defines constants for a transaction with \a TYPE and \a VERSION.
#define DEFINE_TRANSACTION_CONSTANTS(TYPE, VERSION) \
	/* Transaction format version. */ \
	static constexpr uint8_t Current_Version = VERSION; \
	/* Transaction type. */ \
	static constexpr EntityType Entity_Type = TYPE;

/// Defines \a NAME (\a TYPE typed) variable data accessors around a similarly named templated untyped data accessor.
#define DEFINE_TRANSACTION_VARIABLE_DATA_ACCESSORS(NAME, TYPE) \
	/* Returns a const pointer to the typed data contained in this transaction. */ \
	const TYPE* NAME##Ptr() const { \
		return reinterpret_cast<const TYPE*>(NAME##PtrT(*this)); \
	} \
	\
	/* Returns a pointer to the typed data contained in this transaction. */ \
	TYPE* NAME##Ptr() { \
		return reinterpret_cast<TYPE*>(NAME##PtrT(*this)); \
	}

/// Defines a transaction with \a NAME that supports embedding.
#define DEFINE_EMBEDDABLE_TRANSACTION(NAME) \
	struct Embedded##NAME##Transaction : public NAME##TransactionBody<model::EmbeddedTransaction> {}; \
	struct NAME##Transaction : public NAME##TransactionBody<model::Transaction> {};

	// endregion
}}

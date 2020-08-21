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
#include "catapult/plugins.h"

namespace catapult { namespace model { class TransactionRegistry; } }

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a transaction.
	struct PLUGIN_API_DEPENDENCY Transaction : public VerifiableEntity {
		/// Maximum transaction fee paid for confirmation.
		/// \note Actual fee paid is dependent on containing block.
		Amount MaxFee;

		/// Transaction deadline.
		Timestamp Deadline;
	};

	// Transaction Layout:
	// * SizePrefixedEntity
	//   0x00:  (4) Size
	// * VerifiableEntity
	//   0x04:  (4) VerifiableEntityHeader_Reserved1
	//   0x08: (64) Signature
	// * EntityBody
	//   0x48: (32) SignerPublicKey
	//   0x68:  (4) EntityBody_Reserved1
	//   0x6C:  (1) Version
	//   0x6D:  (1) Network
	//   0x6E:  (2) Type
	// * Transaction
	//   0x70:  (8) MaxFee
	//   0x78:  (8) Deadline
	//   0x80:  (*) Transaction Data

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
#define DEFINE_TRANSACTION_VARIABLE_DATA_ACCESSORS DEFINE_SIZE_PREFIXED_ENTITY_VARIABLE_DATA_ACCESSORS

/// Defines a transaction with \a NAME that supports embedding.
#define DEFINE_EMBEDDABLE_TRANSACTION(NAME) \
	struct Embedded##NAME##Transaction : public NAME##TransactionBody<model::EmbeddedTransaction> {}; \
	struct NAME##Transaction : public NAME##TransactionBody<model::Transaction> {};

	// endregion
}}

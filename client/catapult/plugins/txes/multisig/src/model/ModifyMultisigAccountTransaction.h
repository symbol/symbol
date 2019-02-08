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
#include "MultisigEntityType.h"
#include "catapult/model/Transaction.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Cosignatory modification type.
	enum class CosignatoryModificationType : uint8_t {
		/// Add cosignatory.
		Add,

		/// Remove cosignatory.
		Del
	};

	/// Binary layout for cosignatory modification.
	struct CosignatoryModification {
	public:
		/// Modification type.
		CosignatoryModificationType ModificationType;

		/// Cosignatory account public key.
		Key CosignatoryPublicKey;
	};

	/// Binary layout for a modify multisig account transaction body.
	template<typename THeader>
	struct ModifyMultisigAccountTransactionBody : public THeader {
	private:
		using TransactionType = ModifyMultisigAccountTransactionBody<THeader>;

	public:
		DEFINE_TRANSACTION_CONSTANTS(Entity_Type_Modify_Multisig_Account, 3)

	public:
		/// Relative change of the minimal number of cosignatories required when removing an account.
		int8_t MinRemovalDelta;

		/// Relative change of the minimal number of cosignatories required when approving a transaction.
		int8_t MinApprovalDelta;

		/// Number of modifications.
		uint8_t ModificationsCount;

		// followed by modifications data if ModificationsCount != 0
		DEFINE_TRANSACTION_VARIABLE_DATA_ACCESSORS(Modifications, CosignatoryModification)

	private:
		template<typename T>
		static auto* ModificationsPtrT(T& transaction) {
			return transaction.ModificationsCount ? THeader::PayloadStart(transaction) : nullptr;
		}

	public:
		// Calculates the real size of a modify multisig account \a transaction.
		static constexpr uint64_t CalculateRealSize(const TransactionType& transaction) noexcept {
			return sizeof(TransactionType) + transaction.ModificationsCount * sizeof(CosignatoryModification);
		}
	};

	DEFINE_EMBEDDABLE_TRANSACTION(ModifyMultisigAccount)

#pragma pack(pop)
}}

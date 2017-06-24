#pragma once
#include "catapult/model/EntityType.h"
#include "catapult/model/Transaction.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// The cosignatory modification type.
	enum class CosignatoryModificationType : uint8_t {
		/// Add cosignatory.
		Add = 1,

		/// Remove cosignatory.
		Del = 2
	};

	/// Binary layout for cosignatory modification.
	struct CosignatoryModification {
	public:
		/// The modification type.
		CosignatoryModificationType ModificationType;

		/// The cosignatory account public key.
		Key CosignatoryPublicKey;
	};

	/// Binary layout for a modify multisig account transaction body.
	template<typename THeader>
	struct ModifyMultisigAccountTransactionBody : public THeader {
	private:
		using TransactionType = ModifyMultisigAccountTransactionBody<THeader>;

	public:
		/// Transaction format version.
		static constexpr uint8_t Current_Version = 3;

		/// Transaction type.
		static constexpr EntityType Entity_Type = EntityType::Modify_Multisig_Account;

	public:
		/// The relative change of the minimal number of cosignatories required when removing an account.
		int8_t MinRemovalDelta;

		/// The relative change of the minimal number of cosignatories required when approving a transaction.
		int8_t MinApprovalDelta;

		/// The number of modifications.
		uint8_t ModificationsCount;

		// followed by modifications data if ModificationsCount != 0

	private:
		template<typename T>
		static auto ModificationsPtrT(T& transaction) {
			return transaction.ModificationsCount ? THeader::PayloadStart(transaction) : nullptr;
		}

	public:
		/// Returns a const pointer to the first modification contained in this transaction.
		const CosignatoryModification* ModificationsPtr() const {
			return reinterpret_cast<const CosignatoryModification*>(ModificationsPtrT(*this));
		}

		/// Returns a pointer to the first modification contained in this transaction.
		CosignatoryModification* ModificationsPtr() {
			return reinterpret_cast<CosignatoryModification*>(ModificationsPtrT(*this));
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

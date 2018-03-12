#pragma once
#include "LockEntityType.h"
#include "LockTypes.h"
#include "catapult/model/Transaction.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a secret proof transaction body.
	template<typename THeader>
	struct SecretProofTransactionBody : public THeader {
	private:
		using TransactionType = SecretProofTransactionBody<THeader>;

	public:
		/// Transaction format version.
		static constexpr uint8_t Current_Version = 1;

		/// Transaction type.
		static constexpr EntityType Entity_Type = Entity_Type_Secret_Proof;

	public:
		/// The hash algorithm.
		LockHashAlgorithm HashAlgorithm;

		/// The secret.
		Hash512 Secret;

		/// The proof size in bytes.
		uint16_t ProofSize;

		// followed by proof data if ProofSize != 0

	private:
		template<typename T>
		static auto* ProofPtrT(T& transaction) {
			return transaction.ProofSize ? THeader::PayloadStart(transaction) : nullptr;
		}

	public:
		/// Returns a const pointer to the proof contained in this transaction.
		const uint8_t* ProofPtr() const {
			return ProofPtrT(*this);
		}

		/// Returns a pointer to the proof contained in this transaction.
		uint8_t* ProofPtr() {
			return ProofPtrT(*this);
		}

	public:
		// Calculates the real size of secret proof \a transaction.
		static constexpr uint64_t CalculateRealSize(const TransactionType& transaction) noexcept {
			return sizeof(TransactionType) + transaction.ProofSize;
		}
	};

	DEFINE_EMBEDDABLE_TRANSACTION(SecretProof)

#pragma pack(pop)
}}

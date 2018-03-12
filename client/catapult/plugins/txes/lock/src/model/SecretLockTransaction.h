#pragma once
#include "LockEntityType.h"
#include "LockTypes.h"
#include "catapult/model/Mosaic.h"
#include "catapult/model/Transaction.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a secret lock transaction body.
	template<typename THeader>
	struct SecretLockTransactionBody : public THeader {
	private:
		using TransactionType = SecretLockTransactionBody<THeader>;

	public:
		/// Transaction format version.
		static constexpr uint8_t Current_Version = 1;

		/// Transaction type.
		static constexpr EntityType Entity_Type = Entity_Type_Secret_Lock;

	public:
		/// Transaction mosaic.
		model::Mosaic Mosaic;

		/// The number of blocks for which a lock should be valid.
		BlockDuration Duration;

		/// The hash algorithm.
		LockHashAlgorithm HashAlgorithm;

		/// The secret.
		Hash512 Secret;

		/// The recipient of the locked mosaic.
		Address Recipient;

	public:
		// Calculates the real size of secret lock \a transaction.
		static constexpr uint64_t CalculateRealSize(const TransactionType&) noexcept {
			return sizeof(TransactionType);
		}
	};

	DEFINE_EMBEDDABLE_TRANSACTION(SecretLock)

#pragma pack(pop)
}}

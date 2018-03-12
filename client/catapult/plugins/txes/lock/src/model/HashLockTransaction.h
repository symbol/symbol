#pragma once
#include "LockEntityType.h"
#include "catapult/model/Mosaic.h"
#include "catapult/model/Transaction.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a hash lock transaction body.
	template<typename THeader>
	struct HashLockTransactionBody : public THeader {
	private:
		using TransactionType = HashLockTransactionBody<THeader>;

	public:
		/// Transaction format version.
		static constexpr uint8_t Current_Version = 1;

		/// Transaction type.
		static constexpr EntityType Entity_Type = Entity_Type_Hash_Lock;

	public:
		/// Transaction mosaic.
		model::Mosaic Mosaic;

		/// The number of blocks for which a lock should be valid.
		BlockDuration Duration;

		/// Lock hash.
		Hash256 Hash;

	public:
		// Calculates the real size of hash lock \a transaction.
		static constexpr uint64_t CalculateRealSize(const TransactionType&) noexcept {
			return sizeof(TransactionType);
		}
	};

	DEFINE_EMBEDDABLE_TRANSACTION(HashLock)

#pragma pack(pop)
}}

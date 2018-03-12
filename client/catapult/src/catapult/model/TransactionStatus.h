#pragma once
#include "catapult/types.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// A transaction status.
	struct TransactionStatus {
	public:
		/// Creates a TransactionStatus around \a hash, \a status and \a deadline.
		explicit TransactionStatus(const Hash256& hash, uint32_t status, catapult::Timestamp deadline)
				: Hash(hash)
				, Status(status)
				, Deadline(deadline)
		{}

	public:
		/// The transaction hash.
		Hash256 Hash;

		/// The status.
		uint32_t Status;

		/// The deadline.
		catapult::Timestamp Deadline;

	public:
		/// Returns \c true if this transaction status is equal to \a rhs.
		bool operator==(const TransactionStatus& rhs) const {
			return Hash == rhs.Hash;
		}

		/// Returns \c true if this transaction status is not equal to \a rhs.
		bool operator!=(const TransactionStatus& rhs) const {
			return !(*this == rhs);
		}
	};

#pragma pack(pop)
}}

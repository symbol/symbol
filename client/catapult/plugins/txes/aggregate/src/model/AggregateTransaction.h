#pragma once
#include "Cosignature.h"
#include "catapult/model/EntityType.h"
#include "catapult/model/Transaction.h"
#include "catapult/model/TransactionContainer.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for an aggregate transaction header.
	struct AggregateTransactionHeader : public Transaction {
	public:
		/// Transaction format version.
		static constexpr uint8_t Current_Version = 2;

		/// Transaction type.
		static constexpr EntityType Entity_Type = EntityType::Aggregate;

	public:
		/// The transaction payload size in bytes.
		/// \note This is the total number bytes occupied by all sub-transactions.
		uint32_t PayloadSize;

		// followed by sub-transaction data
		// followed by cosignatures data
	};

	/// Binary layout for an aggregate transaction.
	struct AggregateTransaction : public TransactionContainer<AggregateTransactionHeader, EmbeddedEntity> {
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
		/// Returns a const pointer to the first cosignature contained in this transaction.
		/// \note The returned pointer is undefined if the aggregate has an invalid size.
		const Cosignature* CosignaturesPtr() const {
			return reinterpret_cast<const Cosignature*>(CosignaturesPtrT(*this));
		}

		/// Returns a pointer to the first cosignature contained in this transaction.
		/// \note The returned pointer is undefined if the aggregate has an invalid size.
		Cosignature* CosignaturesPtr() {
			return reinterpret_cast<Cosignature*>(CosignaturesPtrT(*this));
		}

		/// Returns the number of cosignatures attached to this transaction.
		/// \note The returned value is undefined if the aggregate has an invalid size.
		size_t CosignaturesCount() const {
			return CosignaturesCountT(*this);
		}

		/// Returns the number of cosignatures attached to this transaction.
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

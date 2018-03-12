#pragma once
#include "TransferEntityType.h"
#include "catapult/model/Mosaic.h"
#include "catapult/model/Transaction.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a transfer transaction body.
	template<typename THeader>
	struct TransferTransactionBody : public THeader {
	private:
		using TransactionType = TransferTransactionBody<THeader>;

	public:
		/// Transaction format version.
		static constexpr uint8_t Current_Version = 3;

		/// Transaction type.
		static constexpr EntityType Entity_Type = Entity_Type_Transfer;

	public:
		/// The transaction recipient.
		Address Recipient;

		/// The message size in bytes.
		uint16_t MessageSize;

		/// The number of mosaics.
		uint8_t MosaicsCount;

		// followed by message data if MessageSize != 0
		// followed by mosaics data if MosaicsCount != 0

	private:
		template<typename T>
		static auto* MessagePtrT(T& transaction) {
			return transaction.MessageSize ? THeader::PayloadStart(transaction) : nullptr;
		}

		template<typename T>
		static auto* MosaicsPtrT(T& transaction) {
			auto* pPayloadStart = THeader::PayloadStart(transaction);
			return transaction.MosaicsCount && pPayloadStart ? pPayloadStart + transaction.MessageSize : nullptr;
		}

	public:
		/// Returns a const pointer to the message contained in this transaction.
		const uint8_t* MessagePtr() const {
			return MessagePtrT(*this);
		}

		/// Returns a pointer to the message contained in this transaction.
		uint8_t* MessagePtr() {
			return MessagePtrT(*this);
		}

		/// Returns a const pointer to the first mosaic contained in this transaction.
		const Mosaic* MosaicsPtr() const {
			return reinterpret_cast<const Mosaic*>(MosaicsPtrT(*this));
		}

		/// Returns a pointer to the first mosaic contained in this transaction.
		Mosaic* MosaicsPtr() {
			return reinterpret_cast<Mosaic*>(MosaicsPtrT(*this));
		}

	public:
		// Calculates the real size of transfer \a transaction.
		static constexpr uint64_t CalculateRealSize(const TransactionType& transaction) noexcept {
			return sizeof(TransactionType) + transaction.MessageSize + transaction.MosaicsCount * sizeof(Mosaic);
		}
	};

	DEFINE_EMBEDDABLE_TRANSACTION(Transfer)

#pragma pack(pop)
}}

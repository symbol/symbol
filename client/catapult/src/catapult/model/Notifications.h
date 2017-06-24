#pragma once
#include "NetworkInfo.h"
#include "NotificationType.h"
#include "catapult/types.h"

namespace catapult { namespace model {

	/// A basic notification.
	struct Notification {
	public:
		/// Creates a new notification with \a type and \a size.
		explicit Notification(NotificationType type, size_t size)
				: Type(type)
				, Size(size)
		{}

	public:
		/// The notification type.
		NotificationType Type;

		/// The notification size.
		size_t Size;
	};

	// region account

	/// Notification of use of an account address.
	struct AccountAddressNotification : public Notification {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Core_Register_Account_Address_Notification;

	public:
		/// Creates a notification around \a address.
		explicit AccountAddressNotification(const Address& address)
				: Notification(Notification_Type, sizeof(AccountAddressNotification))
				, Address(address)
		{}

	public:
		/// The address.
		const catapult::Address& Address;
	};

	/// Notification of use of an account public key.
	struct AccountPublicKeyNotification : public Notification {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Core_Register_Account_Public_Key_Notification;

	public:
		/// Creates a notification around \a publicKey.
		explicit AccountPublicKeyNotification(const Key& publicKey)
				: Notification(Notification_Type, sizeof(AccountPublicKeyNotification))
				, PublicKey(publicKey)
		{}

	public:
		/// The public key.
		const Key& PublicKey;
	};

	// endregion

	// region balance

	/// Notifies a balance transfer from sender to recipient.
	struct BalanceTransferNotification : public Notification {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Core_Balance_Transfer_Notification;

	public:
		/// Creates a notification around \a sender, \a recipient, \a mosaicId and \a amount.
		explicit BalanceTransferNotification(const Key& sender, const Address& recipient, MosaicId mosaicId, Amount amount)
				: Notification(Notification_Type, sizeof(BalanceTransferNotification))
				, Sender(sender)
				, Recipient(recipient)
				, MosaicId(mosaicId)
				, Amount(amount)
		{}

	public:
		/// The sender.
		const Key& Sender;

		/// The recipient.
		const Address& Recipient;

		/// The mosaic id.
		catapult::MosaicId MosaicId;

		/// The amount.
		catapult::Amount Amount;
	};

	// endregion

	// region entity

	/// Notifies the arrival of an entity.
	struct EntityNotification : public Notification {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Core_Entity_Notification;

	public:
		/// Creates an entity notification around \a networkIdentifier.
		explicit EntityNotification(model::NetworkIdentifier networkIdentifier)
				: Notification(Notification_Type, sizeof(EntityNotification))
				, NetworkIdentifier(networkIdentifier)
		{}

	public:
		/// The network identifier.
		model::NetworkIdentifier NetworkIdentifier;
	};

	// endregion

	// region block

	/// Notifies the arrival of a block.
	struct BlockNotification : public Notification {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Core_Block_Notification;

	public:
		/// Creates a block notification around \a signer, \a timestamp and \a difficulty.
		explicit BlockNotification(const Key& signer, Timestamp timestamp, Difficulty difficulty)
				: Notification(Notification_Type, sizeof(BlockNotification))
				, Signer(signer)
				, Timestamp(timestamp)
				, Difficulty(difficulty)
				, NumTransactions(0)
		{}

	public:
		/// The block signer.
		const Key& Signer;

		/// The block timestamp.
		catapult::Timestamp Timestamp;

		/// The block difficulty.
		catapult::Difficulty Difficulty;

		/// The total block fee.
		Amount TotalFee;

		/// The number of block transactions.
		uint32_t NumTransactions;
	};

	// endregion

	// region transaction

	/// Notifies the arrival of a transaction.
	struct TransactionNotification : public Notification {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Core_Transaction_Notification;

	public:
		/// Creates a transaction notification around \a signer, \a entityHash and \a deadline.
		explicit TransactionNotification(const Key& signer, const Hash256& entityHash, Timestamp deadline)
				: Notification(Notification_Type, sizeof(TransactionNotification))
				, Signer(signer)
				, EntityHash(entityHash)
				, Deadline(deadline)
		{}

	public:
		/// The transaction signer.
		const Key& Signer;

		/// The transaction hash.
		const Hash256& EntityHash;

		/// The transaction deadline.
		Timestamp Deadline;
	};

	// endregion

	// region signature

	/// Notifies the presence of a signature.
	struct SignatureNotification : public Notification {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Core_Signature_Notification;

	public:
		/// Creates a signature notification around \a signer, \a signature and \a data.
		explicit SignatureNotification(const Key& signer, const Signature& signature, const RawBuffer& data)
				: Notification(Notification_Type, sizeof(SignatureNotification))
				, Signer(signer)
				, Signature(signature)
				, Data(data)
		{}

	public:
		/// The signer.
		const Key& Signer;

		/// The signature.
		const catapult::Signature& Signature;

		/// The signed data.
		RawBuffer Data;
	};

	// endregion
}}

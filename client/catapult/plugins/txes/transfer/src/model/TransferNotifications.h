#pragma once
#include "catapult/model/Mosaic.h"
#include "catapult/model/Notifications.h"

namespace catapult { namespace model {

	// region transfer notification types

/// Defines a transfer notification type with \a DESCRIPTION, \a CODE and \a CHANNEL.
#define DEFINE_TRANSFER_NOTIFICATION(DESCRIPTION, CODE, CHANNEL) DEFINE_NOTIFICATION_TYPE(CHANNEL, Transfer, DESCRIPTION, CODE)

	/// Transfer was received with a message.
	DEFINE_TRANSFER_NOTIFICATION(Message, 0x001, Validator);

	/// Transfer was received with at least one mosaic.
	DEFINE_TRANSFER_NOTIFICATION(Mosaics, 0x002, Validator);

#undef DEFINE_TRANSFER_NOTIFICATION

	// endregion

	/// Notification of a transfer transaction with a message.
	struct TransferMessageNotification : public Notification {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Transfer_Message_Notification;

	public:
		/// Creates a notification around \a messageSize.
		explicit TransferMessageNotification(uint16_t messageSize)
				: Notification(Notification_Type, sizeof(TransferMessageNotification))
				, MessageSize(messageSize)
		{}

	public:
		/// The message size in bytes.
		uint16_t MessageSize;
	};

	/// Notification of a transfer transaction with mosaics.
	struct TransferMosaicsNotification : public Notification {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Transfer_Mosaics_Notification;

	public:
		/// Creates a notification around \a mosaicsCount and \a pMosaics.
		explicit TransferMosaicsNotification(uint8_t mosaicsCount, const Mosaic* pMosaics)
				: Notification(Notification_Type, sizeof(TransferMosaicsNotification))
				, MosaicsCount(mosaicsCount)
				, MosaicsPtr(pMosaics)
		{}

	public:
		/// The number of mosaics.
		uint8_t MosaicsCount;

		/// Const pointer to the first mosaic.
		const Mosaic* MosaicsPtr;
	};
}}

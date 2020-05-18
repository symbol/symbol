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
#include "MosaicConstants.h"
#include "MosaicProperties.h"
#include "MosaicTypes.h"
#include "catapult/model/Notifications.h"

namespace catapult { namespace model {

	// region mosaic notification types

/// Defines a mosaic notification type with \a DESCRIPTION, \a CODE and \a CHANNEL.
#define DEFINE_MOSAIC_NOTIFICATION(DESCRIPTION, CODE, CHANNEL) DEFINE_NOTIFICATION_TYPE(CHANNEL, Mosaic, DESCRIPTION, CODE)

	/// Mosaic properties were provided.
	DEFINE_MOSAIC_NOTIFICATION(Properties, 0x0001, Validator);

	/// Mosaic was defined.
	DEFINE_MOSAIC_NOTIFICATION(Definition, 0x0002, All);

	/// Mosaic nonce and id were provided.
	DEFINE_MOSAIC_NOTIFICATION(Nonce, 0x0003, Validator);

	/// Mosaic supply was changed.
	DEFINE_MOSAIC_NOTIFICATION(Supply_Change, 0x0004, All);

	/// Mosaic rental fee has been sent.
	DEFINE_MOSAIC_NOTIFICATION(Rental_Fee, 0x0005, Observer);

#undef DEFINE_MOSAIC_NOTIFICATION

	// endregion

	// region MosaicPropertiesNotification

	/// Notification of mosaic properties.
	struct MosaicPropertiesNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Mosaic_Properties_Notification;

	public:
		/// Creates a notification around \a properties.
		explicit MosaicPropertiesNotification(const MosaicProperties& properties)
				: Notification(Notification_Type, sizeof(MosaicPropertiesNotification))
				, Properties(properties)
		{}

	public:
		/// Mosaic properties.
		MosaicProperties Properties;
	};

	// endregion

	// region MosaicDefinitionNotification

	/// Notification of a mosaic definition.
	struct MosaicDefinitionNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Mosaic_Definition_Notification;

	public:
		/// Creates a notification around \a owner, \a mosaicId and \a properties.
		MosaicDefinitionNotification(const Key& owner, MosaicId mosaicId, const MosaicProperties& properties)
				: Notification(Notification_Type, sizeof(MosaicDefinitionNotification))
				, Owner(owner)
				, MosaicId(mosaicId)
				, Properties(properties)
		{}

	public:
		/// Mosaic owner.
		const Key& Owner;

		/// Id of the mosaic.
		catapult::MosaicId MosaicId;

		/// Mosaic properties.
		MosaicProperties Properties;
	};

	// endregion

	// region MosaicNonceNotification

	/// Notification of a mosaic nonce and id.
	struct MosaicNonceNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Mosaic_Nonce_Notification;

	public:
		/// Creates a notification around \a owner, \a mosaicNonce and \a mosaicId.
		MosaicNonceNotification(const Key& owner, MosaicNonce mosaicNonce, catapult::MosaicId mosaicId)
				: Notification(Notification_Type, sizeof(MosaicNonceNotification))
				, Owner(owner)
				, MosaicNonce(mosaicNonce)
				, MosaicId(mosaicId)
		{}

	public:
		/// Mosaic owner.
		const Key& Owner;

		/// Mosaic nonce.
		catapult::MosaicNonce MosaicNonce;

		/// Mosaic id.
		catapult::MosaicId MosaicId;
	};

	// endregion

	// region MosaicSupplyChangeNotification

	/// Notification of a mosaic supply change.
	struct MosaicSupplyChangeNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Mosaic_Supply_Change_Notification;

	public:
		/// Creates a notification around \a owner, \a mosaicId, \a action and \a delta.
		MosaicSupplyChangeNotification(const Key& owner, UnresolvedMosaicId mosaicId, MosaicSupplyChangeAction action, Amount delta)
				: Notification(Notification_Type, sizeof(MosaicSupplyChangeNotification))
				, Owner(owner)
				, MosaicId(mosaicId)
				, Action(action)
				, Delta(delta)
		{}

	public:
		/// Mosaic owner.
		const Key& Owner;

		/// Id of the affected mosaic.
		UnresolvedMosaicId MosaicId;

		/// Supply change action.
		MosaicSupplyChangeAction Action;

		/// Amount of the change.
		Amount Delta;
	};

	// endregion

	// region rental MosaicRentalFeeNotification

	/// Notification of a mosaic rental fee.
	struct MosaicRentalFeeNotification : public BasicBalanceNotification<MosaicRentalFeeNotification> {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Mosaic_Rental_Fee_Notification;

	public:
		/// Creates a notification around \a sender, \a recipient, \a mosaicId and \a amount.
		MosaicRentalFeeNotification(
				const Key& sender,
				const UnresolvedAddress& recipient,
				UnresolvedMosaicId mosaicId,
				catapult::Amount amount)
				: BasicBalanceNotification(sender, mosaicId, amount)
				, Recipient(recipient)
		{}

	public:
		/// Recipient.
		UnresolvedAddress Recipient;
	};

	// endregion
}}

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
	DEFINE_MOSAIC_NOTIFICATION(Properties, 0x0012, Validator);

	/// Mosaic was defined.
	DEFINE_MOSAIC_NOTIFICATION(Definition, 0x0013, All);

	/// Mosaic nonce and id were provided.
	DEFINE_MOSAIC_NOTIFICATION(Nonce, 0x0014, Validator);

	/// Mosaic supply was changed.
	DEFINE_MOSAIC_NOTIFICATION(Supply_Change, 0x0022, All);

	/// Mosaic rental fee has been sent.
	DEFINE_MOSAIC_NOTIFICATION(Rental_Fee, 0x0030, Observer);

#undef DEFINE_MOSAIC_NOTIFICATION

	// endregion

	// region MosaicPropertiesNotification

	/// Notification of mosaic properties.
	/// \note This is required due to potentially lossy conversion from raw properties to MosaicProperties.
	struct MosaicPropertiesNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Mosaic_Properties_Notification;

	public:
		/// Creates a notification around \a propertiesHeader and \a pProperties.
		MosaicPropertiesNotification(const MosaicPropertiesHeader& propertiesHeader, const MosaicProperty* pProperties)
				: Notification(Notification_Type, sizeof(MosaicPropertiesNotification))
				, PropertiesHeader(propertiesHeader)
				, PropertiesPtr(pProperties)
		{}

	public:
		/// Mosaic properties header.
		const MosaicPropertiesHeader& PropertiesHeader;

		/// Const pointer to the optional properties.
		const MosaicProperty* PropertiesPtr;
	};

	// endregion

	// region MosaicDefinitionNotification

	/// Notification of a mosaic definition.
	struct MosaicDefinitionNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Mosaic_Definition_Notification;

	public:
		/// Creates a notification around \a signer, \a mosaicId and \a properties.
		MosaicDefinitionNotification(const Key& signer, MosaicId mosaicId, const MosaicProperties& properties)
				: Notification(Notification_Type, sizeof(MosaicDefinitionNotification))
				, Signer(signer)
				, MosaicId(mosaicId)
				, Properties(properties)
		{}

	public:
		/// Signer.
		const Key& Signer;

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
		/// Creates a notification around \a signer, \a mosaicNonce and \a mosaicId.
		MosaicNonceNotification(const Key& signer, MosaicNonce mosaicNonce, catapult::MosaicId mosaicId)
				: Notification(Notification_Type, sizeof(MosaicNonceNotification))
				, Signer(signer)
				, MosaicNonce(mosaicNonce)
				, MosaicId(mosaicId)
		{}

	public:
		/// Signer.
		const Key& Signer;

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
		/// Creates a notification around \a signer, \a mosaicId, \a direction and \a delta.
		MosaicSupplyChangeNotification(const Key& signer, UnresolvedMosaicId mosaicId, MosaicSupplyChangeDirection direction, Amount delta)
				: Notification(Notification_Type, sizeof(MosaicSupplyChangeNotification))
				, Signer(signer)
				, MosaicId(mosaicId)
				, Direction(direction)
				, Delta(delta)
		{}

	public:
		/// Signer.
		const Key& Signer;

		/// Id of the affected mosaic.
		UnresolvedMosaicId MosaicId;

		/// Supply change direction.
		MosaicSupplyChangeDirection Direction;

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

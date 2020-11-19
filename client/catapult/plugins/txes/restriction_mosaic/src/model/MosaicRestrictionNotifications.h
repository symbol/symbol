/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "src/model/MosaicRestrictionTypes.h"
#include "catapult/model/Notifications.h"

namespace catapult { namespace model {

	// region mosaic restriction notification types

/// Defines a mosaic restriction notification type with \a DESCRIPTION, \a CODE and \a CHANNEL.
#define DEFINE_MOSAIC_RESTRICTION_NOTIFICATION(DESCRIPTION, CODE, CHANNEL) \
	DEFINE_NOTIFICATION_TYPE(CHANNEL, RestrictionMosaic, DESCRIPTION, CODE)

	/// Mosaic restriction type.
	DEFINE_MOSAIC_RESTRICTION_NOTIFICATION(Type, 0x0001, Validator);

	/// Mosaic restriction is required to exist.
	DEFINE_MOSAIC_RESTRICTION_NOTIFICATION(Required, 0x0002, Validator);

	/// Mosaic global restriction modification (previous value).
	DEFINE_MOSAIC_RESTRICTION_NOTIFICATION(Global_Restriction_Previous, 0x0003, All);

	/// Mosaic global restriction modification (new value).
	DEFINE_MOSAIC_RESTRICTION_NOTIFICATION(Global_Restriction_New, 0x0004, All);

	/// Mosaic address restriction modification (previous value).
	DEFINE_MOSAIC_RESTRICTION_NOTIFICATION(Address_Restriction_Previous, 0x0005, All);

	/// Mosaic address restriction modification (new value).
	DEFINE_MOSAIC_RESTRICTION_NOTIFICATION(Address_Restriction_New, 0x0006, All);

#undef DEFINE_MOSAIC_RESTRICTION_NOTIFICATION

	// endregion

	// region MosaicRestrictionTypeNotification

	/// Notification of a mosaic restriction type.
	struct MosaicRestrictionTypeNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = RestrictionMosaic_Type_Notification;

	public:
		/// Creates a notification around \a restrictionType.
		explicit MosaicRestrictionTypeNotification(model::MosaicRestrictionType restrictionType)
				: Notification(Notification_Type, sizeof(MosaicRestrictionTypeNotification))
				, RestrictionType(restrictionType)
		{}

	public:
		/// Mosaic restriction type.
		MosaicRestrictionType RestrictionType;
	};

	// endregion

	// region MosaicRestrictionRequiredNotification

	/// Notification of a required mosaic restriction.
	struct MosaicRestrictionRequiredNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = RestrictionMosaic_Required_Notification;

	public:
		/// Creates a notification around \a mosaicId and \a restrictionKey.
		MosaicRestrictionRequiredNotification(UnresolvedMosaicId mosaicId, uint64_t restrictionKey)
				: Notification(Notification_Type, sizeof(MosaicRestrictionRequiredNotification))
				, MosaicId(mosaicId)
				, RestrictionKey(restrictionKey)
		{}

	public:
		/// Identifier of the mosaic being restricted.
		UnresolvedMosaicId MosaicId;

		/// Restriction key relative to the mosaic id.
		uint64_t RestrictionKey;
	};

	// endregion

	// region MosaicGlobalRestrictionModificationNotification

	/// Notification of a mosaic global restriction modification.
	template<NotificationType NotificationType>
	struct MosaicGlobalRestrictionModificationNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = NotificationType;

	public:
		/// Creates a notification around \a mosaicId, \a referenceMosaicId, \a restrictionKey, \a restrictionValue and \a restrictionType.
		MosaicGlobalRestrictionModificationNotification(
				UnresolvedMosaicId mosaicId,
				UnresolvedMosaicId referenceMosaicId,
				uint64_t restrictionKey,
				uint64_t restrictionValue,
				MosaicRestrictionType restrictionType)
				: Notification(Notification_Type, sizeof(MosaicGlobalRestrictionModificationNotification))
				, MosaicId(mosaicId)
				, ReferenceMosaicId(referenceMosaicId)
				, RestrictionKey(restrictionKey)
				, RestrictionValue(restrictionValue)
				, RestrictionType(restrictionType)
		{}

	public:
		/// Identifier of the mosaic being restricted.
		UnresolvedMosaicId MosaicId;

		/// Identifier of the mosaic providing the restriction key.
		UnresolvedMosaicId ReferenceMosaicId;

		/// Restriction key relative to the reference mosaic id.
		uint64_t RestrictionKey;

		/// Restriction value.
		uint64_t RestrictionValue;

		/// Restriction type.
		MosaicRestrictionType RestrictionType;
	};

	/// First notification of a mosaic global restriction modification composed of previous values.
	using MosaicGlobalRestrictionModificationPreviousValueNotification =
		MosaicGlobalRestrictionModificationNotification<RestrictionMosaic_Global_Restriction_Previous_Notification>;

	/// Second notification of a mosaic global restriction modification composed of new values.
	using MosaicGlobalRestrictionModificationNewValueNotification =
		MosaicGlobalRestrictionModificationNotification<RestrictionMosaic_Global_Restriction_New_Notification>;

	// endregion

	// region MosaicAddressRestrictionModificationNotification

	/// Notification of a mosaic address restriction modification.
	template<NotificationType NotificationType>
	struct MosaicAddressRestrictionModificationNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = NotificationType;

	public:
		/// Creates a notification around \a mosaicId, \a restrictionKey, \a targetAddress and \a restrictionValue.
		MosaicAddressRestrictionModificationNotification(
				UnresolvedMosaicId mosaicId,
				uint64_t restrictionKey,
				const UnresolvedAddress& targetAddress,
				uint64_t restrictionValue)
				: Notification(Notification_Type, sizeof(MosaicAddressRestrictionModificationNotification))
				, MosaicId(mosaicId)
				, RestrictionKey(restrictionKey)
				, TargetAddress(targetAddress)
				, RestrictionValue(restrictionValue)
		{}

	public:
		/// Identifier of the mosaic to which the restriction applies.
		UnresolvedMosaicId MosaicId;

		/// Restriction key.
		uint64_t RestrictionKey;

		/// Address being restricted.
		const UnresolvedAddress& TargetAddress;

		/// Restriction value.
		uint64_t RestrictionValue;
	};

	/// First notification of a mosaic address restriction modification composed of previous values.
	using MosaicAddressRestrictionModificationPreviousValueNotification =
		MosaicAddressRestrictionModificationNotification<RestrictionMosaic_Address_Restriction_Previous_Notification>;

	/// Second notification of a mosaic address restriction modification composed of new values.
	using MosaicAddressRestrictionModificationNewValueNotification =
		MosaicAddressRestrictionModificationNotification<RestrictionMosaic_Address_Restriction_New_Notification>;

	// endregion
}}

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
#include "MetadataTypes.h"
#include "catapult/model/Notifications.h"

namespace catapult { namespace model {

	// region metadata notification types

/// Defines a metadata notification type with \a DESCRIPTION, \a CODE and \a CHANNEL.
#define DEFINE_METADATA_NOTIFICATION(DESCRIPTION, CODE, CHANNEL) DEFINE_NOTIFICATION_TYPE(CHANNEL, Metadata, DESCRIPTION, CODE)

	/// Metadata value was received with specified sizes.
	DEFINE_METADATA_NOTIFICATION(Sizes, 0x0001, Validator);

	/// Metadata value was received.
	DEFINE_METADATA_NOTIFICATION(Value, 0x0002, All);

#undef DEFINE_METADATA_NOTIFICATION

	// endregion

	// region MetadataSizesNotification

	/// Notification of metadata sizes.
	struct MetadataSizesNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Metadata_Sizes_Notification;

	public:
		/// Creates a notification around \a valueSizeDelta and \a valueSize.
		MetadataSizesNotification(int16_t valueSizeDelta, uint16_t valueSize)
				: Notification(Notification_Type, sizeof(MetadataSizesNotification))
				, ValueSizeDelta(valueSizeDelta)
				, ValueSize(valueSize)
		{}

	public:
		/// Change in value size in bytes.
		int16_t ValueSizeDelta;

		/// Value size in bytes.
		uint16_t ValueSize;
	};

	// endregion

	// region MetadataValueNotification

	/// Notification of metadata value.
	struct MetadataValueNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Metadata_Value_Notification;

	public:
		/// Creates a notification around \a partialMetadataKey, \a metadataTarget, \a valueSizeDelta, \a valueSize and \a pValue.
		MetadataValueNotification(
				const UnresolvedPartialMetadataKey& partialMetadataKey,
				const model::MetadataTarget& metadataTarget,
				int16_t valueSizeDelta,
				uint16_t valueSize,
				const uint8_t* pValue)
				: Notification(Notification_Type, sizeof(MetadataValueNotification))
				, PartialMetadataKey(partialMetadataKey)
				, MetadataTarget(metadataTarget)
				, ValueSizeDelta(valueSizeDelta)
				, ValueSize(valueSize)
				, ValuePtr(pValue)
		{}

	public:
		/// Partial metadata key.
		UnresolvedPartialMetadataKey PartialMetadataKey;

		/// Metadata target.
		model::MetadataTarget MetadataTarget;

		/// Change in value size in bytes.
		int16_t ValueSizeDelta;

		/// Value size in bytes.
		uint16_t ValueSize;

		/// Const pointer to the metadata value.
		const uint8_t* ValuePtr;
	};

	// endregion
}}

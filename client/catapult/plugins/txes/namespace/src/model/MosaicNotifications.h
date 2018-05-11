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
#include "MosaicProperties.h"
#include "MosaicTypes.h"
#include "NamespaceConstants.h"
#include "catapult/model/Notifications.h"

namespace catapult { namespace model {

	// region mosaic notification types

/// Defines a mosaic notification type with \a DESCRIPTION, \a CODE and \a CHANNEL.
#define DEFINE_MOSAIC_NOTIFICATION(DESCRIPTION, CODE, CHANNEL) DEFINE_NOTIFICATION_TYPE(CHANNEL, Mosaic, DESCRIPTION, CODE)

	/// Mosaic name was provided.
	DEFINE_MOSAIC_NOTIFICATION(Name, 0x0011, Validator);

	/// Mosaic properties were provided.
	DEFINE_MOSAIC_NOTIFICATION(Properties, 0x0012, Validator);

	/// Mosaic was defined.
	DEFINE_MOSAIC_NOTIFICATION(Definition, 0x0013, All);

	/// Mosaic was changed.
	DEFINE_MOSAIC_NOTIFICATION(Change, 0x0021, Validator);

	/// Mosaic supply was changed.
	DEFINE_MOSAIC_NOTIFICATION(Supply_Change, 0x0022, All);

#undef DEFINE_MOSAIC_NOTIFICATION

	// endregion

	// region definition

	/// Notification of a mosaic name.
	struct MosaicNameNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Mosaic_Name_Notification;

	public:
		/// Creates a notification around \a nameSize and \a pName given \a mosaicId and \a parentId.
		explicit MosaicNameNotification(catapult::MosaicId mosaicId, NamespaceId parentId, uint8_t nameSize, const uint8_t* pName)
				: Notification(Notification_Type, sizeof(MosaicNameNotification))
				, MosaicId(mosaicId)
				, ParentId(parentId)
				, NameSize(nameSize)
				, NamePtr(pName)
		{}

	public:
		/// Id of the mosaic.
		catapult::MosaicId MosaicId;

		/// Id of the parent namespace.
		NamespaceId ParentId;

		/// Size of the name.
		uint8_t NameSize;

		/// Const pointer to the mosaic name.
		const uint8_t* NamePtr;
	};

	/// Notification of mosaic properties.
	/// \note This is required due to potentially lossy conversion from raw properties to MosaicProperties.
	struct MosaicPropertiesNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Mosaic_Properties_Notification;

	public:
		/// Creates a notification around \a propertiesHeader and \a pProperties.
		explicit MosaicPropertiesNotification(const MosaicPropertiesHeader& propertiesHeader, const MosaicProperty* pProperties)
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

	/// Notification of a mosaic definition.
	struct MosaicDefinitionNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Mosaic_Definition_Notification;

	public:
		/// Creates a notification around \a signer, \a parentId, \a mosaicId and \a properties.
		explicit MosaicDefinitionNotification(
				const Key& signer,
				NamespaceId parentId,
				MosaicId mosaicId,
				const MosaicProperties& properties)
				: Notification(Notification_Type, sizeof(MosaicDefinitionNotification))
				, Signer(signer)
				, ParentId(parentId)
				, MosaicId(mosaicId)
				, Properties(properties)
		{}

	public:
		/// Signer.
		const Key& Signer;

		/// Id of the parent namespace.
		NamespaceId ParentId;

		/// Id of the mosaic.
		catapult::MosaicId MosaicId;

		/// Mosaic properties.
		MosaicProperties Properties;
	};

	// endregion

	// region change

	/// Notification of a mosaic change.
	struct MosaicChangeNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Mosaic_Change_Notification;

	public:
		/// Creates a notification around \a signer and \a mosaicId.
		explicit MosaicChangeNotification(const Key& signer, MosaicId mosaicId)
				: Notification(Notification_Type, sizeof(MosaicChangeNotification))
				, Signer(signer)
				, MosaicId(mosaicId)
		{}

	public:
		/// Signer.
		const Key& Signer;

		/// Id of the affected mosaic.
		catapult::MosaicId MosaicId;
	};

	/// Notification of a mosaic supply change.
	struct MosaicSupplyChangeNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Mosaic_Supply_Change_Notification;

	public:
		/// Creates a notification around \a signer, \a mosaicId, \a direction and \a delta.
		explicit MosaicSupplyChangeNotification(const Key& signer, MosaicId mosaicId, MosaicSupplyChangeDirection direction, Amount delta)
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
		catapult::MosaicId MosaicId;

		/// Supply change direction.
		MosaicSupplyChangeDirection Direction;

		/// Amount of the change.
		Amount Delta;
	};

	// endregion
}}

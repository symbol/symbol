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
#include "src/model/PropertyTypes.h"
#include "src/state/PropertyDescriptor.h"
#include "catapult/model/Notifications.h"

namespace catapult { namespace model {

	// region property notification types

/// Defines a property notification type with \a DESCRIPTION, \a CODE and \a CHANNEL.
#define DEFINE_PROPERTY_NOTIFICATION(DESCRIPTION, CODE, CHANNEL) DEFINE_NOTIFICATION_TYPE(CHANNEL, Property, DESCRIPTION, CODE)

	/// Property type.
	DEFINE_PROPERTY_NOTIFICATION(Type, 0x0001, Validator);

	/// Address property modification.
	DEFINE_PROPERTY_NOTIFICATION(Address_Modification, 0x0010, All);

	/// Mosaic property modification.
	DEFINE_PROPERTY_NOTIFICATION(Mosaic_Modification, 0x0011, All);

	/// Transaction type property modification.
	DEFINE_PROPERTY_NOTIFICATION(Transaction_Type_Modification, 0x0012, All);

	/// Address property modifications.
	DEFINE_PROPERTY_NOTIFICATION(Address_Modifications, 0x0020, Validator);

	/// Mosaic property modifications.
	DEFINE_PROPERTY_NOTIFICATION(Mosaic_Modifications, 0x0021, Validator);

	/// Transaction type property modifications.
	DEFINE_PROPERTY_NOTIFICATION(Transaction_Type_Modifications, 0x0022, Validator);

#undef DEFINE_PROPERTY_NOTIFICATION

	// endregion

	/// Notification of a property type.
	struct PropertyTypeNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Property_Type_Notification;

	public:
		/// Creates a notification around \a propertyType.
		explicit PropertyTypeNotification(model::PropertyType propertyType)
				: Notification(Notification_Type, sizeof(PropertyTypeNotification))
				, PropertyType(propertyType)
		{}

	public:
		/// Property type.
		model::PropertyType PropertyType;
	};

	/// Notification of a property value modification.
	template<typename TPropertyValue, NotificationType Property_Notification_Type>
	struct ModifyPropertyValueNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Property_Notification_Type;

	public:
		/// Creates a notification around \a key, \a propertyType and \a modification.
		explicit ModifyPropertyValueNotification(
				const Key& key,
				PropertyType propertyType,
				const PropertyModification<TPropertyValue>& modification)
				: Notification(Notification_Type, sizeof(ModifyPropertyValueNotification))
				, Key(key)
				, PropertyDescriptor(propertyType)
				, Modification(modification)
		{}

	public:
		/// Account's public key.
		catapult::Key Key;

		/// Property descriptor.
		state::PropertyDescriptor PropertyDescriptor;

		/// Property modification.
		/// \note TPropertyValue is the resolved value.
		PropertyModification<TPropertyValue> Modification;
	};

	using ModifyAddressPropertyValueNotification =
		ModifyPropertyValueNotification<UnresolvedAddress, Property_Address_Modification_Notification>;
	using ModifyMosaicPropertyValueNotification =
		ModifyPropertyValueNotification<UnresolvedMosaicId, Property_Mosaic_Modification_Notification>;
	using ModifyTransactionTypePropertyValueNotification =
		ModifyPropertyValueNotification<EntityType, Property_Transaction_Type_Modification_Notification>;

	/// Notification of a property modification.
	template<typename TPropertyValue, NotificationType Property_Notification_Type>
	struct ModifyPropertyNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Property_Notification_Type;

	public:
		/// Creates a notification around \a key, \a propertyType, \a modificationsCount and \a pModifications.
		explicit ModifyPropertyNotification(
				const Key& key,
				PropertyType propertyType,
				uint8_t modificationsCount,
				const PropertyModification<TPropertyValue>* pModifications)
				: Notification(Notification_Type, sizeof(ModifyPropertyNotification))
				, Key(key)
				, PropertyDescriptor(propertyType)
				, ModificationsCount(modificationsCount)
				, ModificationsPtr(pModifications)
		{}

	public:
		/// Account's public key.
		catapult::Key Key;

		/// Property descriptor.
		state::PropertyDescriptor PropertyDescriptor;

		/// Number of modifications.
		uint8_t ModificationsCount;

		/// Const pointer to the first modification.
		const PropertyModification<TPropertyValue>* ModificationsPtr;
	};

	using ModifyAddressPropertyNotification = ModifyPropertyNotification<UnresolvedAddress, Property_Address_Modifications_Notification>;
	using ModifyMosaicPropertyNotification = ModifyPropertyNotification<UnresolvedMosaicId, Property_Mosaic_Modifications_Notification>;
	using ModifyTransactionTypePropertyNotification = ModifyPropertyNotification<
		EntityType,
		Property_Transaction_Type_Modifications_Notification>;
}}

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
#include "NamespaceConstants.h"
#include "NamespaceTypes.h"
#include "catapult/model/Notifications.h"

namespace catapult { namespace model {

	// region alias notification types

/// Defines a namespace alias notification type with \a DESCRIPTION, \a CODE and \a CHANNEL.
/// \note Alias notifications reuse Namespace facility code.
#define DEFINE_NAMESPACE_ALIAS_NOTIFICATION(DESCRIPTION, CODE, CHANNEL) \
	DEFINE_NOTIFICATION_TYPE(CHANNEL, Namespace, DESCRIPTION, 0x1000 | CODE)

	/// Alias link was provided.
	DEFINE_NAMESPACE_ALIAS_NOTIFICATION(Alias_Link, 0x0001, Validator);

	/// Address alias was un/linked.
	DEFINE_NAMESPACE_ALIAS_NOTIFICATION(Aliased_Address, 0x0002, All);

	/// Mosaic alias was un/linked.
	DEFINE_NAMESPACE_ALIAS_NOTIFICATION(Aliased_MosaicId, 0x0003, All);

#undef DEFINE_NAMESPACE_ALIAS_NOTIFICATION

	// endregion

	// region BaseAliasNotification

	/// Base alias notification.
	struct BaseAliasNotification : public Notification {
	public:
		/// Creates a base alias notification around \a namespaceId and \a aliasAction using \a notificationType and \a notificationSize.
		BaseAliasNotification(
				NotificationType notificationType,
				size_t notificationSize,
				catapult::NamespaceId namespaceId,
				model::AliasAction aliasAction)
				: Notification(notificationType, notificationSize)
				, NamespaceId(namespaceId)
				, AliasAction(aliasAction)
		{}

	public:
		/// Namespace id.
		catapult::NamespaceId NamespaceId;

		/// Alias action.
		model::AliasAction AliasAction;
	};

	// endregion

	// region AliasLinkNotification

	/// Notification of alias link.
	struct AliasLinkNotification : public BaseAliasNotification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Namespace_Alias_Link_Notification;

	public:
		/// Creates a notification around \a namespaceId and \a aliasAction.
		AliasLinkNotification(catapult::NamespaceId namespaceId, model::AliasAction aliasAction)
				: BaseAliasNotification(Notification_Type, sizeof(AliasLinkNotification), namespaceId, aliasAction)
		{}
	};

	// endregion

	// region AliasedDataNotification

	/// Notification of aliased data.
	template<typename TAliasedData, NotificationType Aliased_Notification_Type>
	struct AliasedDataNotification : public BaseAliasNotification {
	private:
		using AliasedNotification = AliasedDataNotification<TAliasedData, Aliased_Notification_Type>;

	public:
		static constexpr auto Notification_Type = Aliased_Notification_Type;

	public:
		/// Creates a notification around \a namespaceId, \a aliasAction and \a aliasedData.
		AliasedDataNotification(catapult::NamespaceId namespaceId, model::AliasAction aliasAction, const TAliasedData& aliasedData)
				: BaseAliasNotification(Notification_Type, sizeof(AliasedNotification), namespaceId, aliasAction)
				, AliasedData(aliasedData)
		{}

	public:
		/// Aliased data.
		TAliasedData AliasedData;
	};

	/// Notification of an aliased address.
	using AliasedAddressNotification = AliasedDataNotification<Address, Namespace_Aliased_Address_Notification>;

	/// Notification of an aliased mosaic id.
	using AliasedMosaicIdNotification = AliasedDataNotification<MosaicId, Namespace_Aliased_MosaicId_Notification>;

	// endregion
}}

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
#include "catapult/model/LinkAction.h"
#include "catapult/model/Mosaic.h"
#include "catapult/model/Notifications.h"

namespace catapult { namespace model {

	// region account link notification types

/// Defines an account link notification type with \a DESCRIPTION, \a CODE and \a CHANNEL.
#define DEFINE_ACCOUNT_LINK_NOTIFICATION(DESCRIPTION, CODE, CHANNEL) DEFINE_NOTIFICATION_TYPE(CHANNEL, AccountLink, DESCRIPTION, CODE)

	/// Remote account was un/linked.
	DEFINE_ACCOUNT_LINK_NOTIFICATION(Remote, 0x0001, All);

	/// New remote account was created.
	DEFINE_ACCOUNT_LINK_NOTIFICATION(New_Remote_Account, 0x0002, Validator);

	/// Account was un/linked to a node.
	DEFINE_ACCOUNT_LINK_NOTIFICATION(Node, 0x0003, All);

#undef DEFINE_ACCOUNTLINK_NOTIFICATION

	// endregion

	// region RemoteAccountKeyLinkNotification

	/// Notification of a remote account key link.
	using RemoteAccountKeyLinkNotification = BasicKeyLinkNotification<Key, AccountLink_Remote_Notification>;

	// endregion

	// region NewRemoteAccountNotification

	/// Notification of a new remote account.
	struct NewRemoteAccountNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = AccountLink_New_Remote_Account_Notification;

	public:
		/// Creates a notification around \a linkedPublicKey.
		explicit NewRemoteAccountNotification(const Key& linkedPublicKey)
				: Notification(Notification_Type, sizeof(NewRemoteAccountNotification))
				, LinkedPublicKey(linkedPublicKey)
		{}

	public:
		/// Linked public key.
		const Key& LinkedPublicKey;
	};

	// endregion

	// region NodeKeyLinkNotification

	/// Notification of a node key link.
	using NodeKeyLinkNotification = BasicKeyLinkNotification<Key, AccountLink_Node_Notification>;

	// endregion
}}

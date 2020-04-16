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
#include "catapult/model/LinkAction.h"
#include "catapult/model/Notifications.h"

namespace catapult { namespace model {

	// region KeyLinkNotification

	/// Notification of a key link.
	template<typename TKey, NotificationType Key_Link_Notification_Type>
	struct BasicKeyLinkNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Key_Link_Notification_Type;

	public:
		/// Creates a notification around \a mainAccountKey, \a linkedPublicKey and \a linkAction.
		BasicKeyLinkNotification(const Key& mainAccountKey, const TKey& linkedPublicKey, LinkAction linkAction)
				: Notification(Notification_Type, sizeof(BasicKeyLinkNotification))
				, MainAccountKey(mainAccountKey)
				, LinkedPublicKey(linkedPublicKey)
				, LinkAction(linkAction)
		{}

	public:
		/// Main account key.
		const Key& MainAccountKey;

		/// Linked public key.
		const TKey& LinkedPublicKey;

		/// Link action.
		model::LinkAction LinkAction;
	};

	// endregion

	/// Notification of a voting key link.
	using VotingKeyLinkNotification = BasicKeyLinkNotification<VotingKey, Core_Voting_Key_Link_Notification>;

	/// Notification of a vrf key link.
	using VrfKeyLinkNotification = BasicKeyLinkNotification<Key, Core_Vrf_Key_Link_Notification>;
}}

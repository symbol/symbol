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
#include "WeakEntityInfo.h"

namespace catapult {
	namespace model {
		class NotificationSubscriber;
		class TransactionRegistry;
	}
}

namespace catapult { namespace model {

	/// Publication modes.
	enum class PublicationMode {
		/// Only basic notifications are published.
		Basic,

		/// Only custom notifications are published.
		Custom,

		/// All notifications are published.
		All
	};

	/// Notification publisher.
	class NotificationPublisher {
	public:
		virtual ~NotificationPublisher() = default;

	public:
		/// Sends all notifications from \a entityInfo to \a sub.
		virtual void publish(const WeakEntityInfo& entityInfo, NotificationSubscriber& sub) const = 0;
	};

	/// Creates a notification publisher around \a transactionRegistry for the specified \a mode given specified
	/// fee mosaic id (\a feeMosaicId).
	std::unique_ptr<NotificationPublisher> CreateNotificationPublisher(
			const TransactionRegistry& transactionRegistry,
			UnresolvedMosaicId feeMosaicId,
			PublicationMode mode = PublicationMode::All);
}}

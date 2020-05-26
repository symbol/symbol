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
#include "catapult/model/Mosaic.h"
#include "catapult/model/Notifications.h"

namespace catapult { namespace model {

	// region BaseLockDurationNotification

	/// Base for lock duration notification.
	template<typename TDerivedNotification>
	struct BaseLockDurationNotification : public Notification {
	public:
		/// Creates a notification around \a duration.
		explicit BaseLockDurationNotification(BlockDuration duration)
				: Notification(TDerivedNotification::Notification_Type, sizeof(TDerivedNotification))
				, Duration(duration)
		{}

	public:
		/// Lock duration.
		BlockDuration Duration;
	};

	// endregion

	// region BaseLockNotification

	/// Base for lock transaction notification.
	template<typename TDerivedNotification>
	struct BaseLockNotification : public Notification {
	protected:
		/// Creates base lock notification around \a owner, \a mosaic and \a duration.
		BaseLockNotification(const Address& owner, const UnresolvedMosaic& mosaic, BlockDuration duration)
				: Notification(TDerivedNotification::Notification_Type, sizeof(TDerivedNotification))
				, Owner(owner)
				, Mosaic(mosaic)
				, Duration(duration)
		{}

	public:
		/// Lock owner.
		Address Owner;

		/// Locked mosaic.
		UnresolvedMosaic Mosaic;

		/// Lock duration.
		BlockDuration Duration;
	};

	// endregion
}}

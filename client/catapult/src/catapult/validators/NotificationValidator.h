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
#include "ValidationResult.h"
// ValidatorContext must be included so that stateful validator has proper visibility
// (all of its template arguments must be visible at point of declaration)
#include "ValidatorContext.h"
#include "catapult/model/Notifications.h"
#include <string>

namespace catapult { namespace validators {

	/// Strongly typed notification validator.
	template<typename TNotification, typename... TArgs>
	class PLUGIN_API_DEPENDENCY NotificationValidatorT {
	public:
		/// Notification type.
		using NotificationType = TNotification;

	public:
		virtual ~NotificationValidatorT() = default;

	public:
		/// Gets the validator name.
		virtual const std::string& name() const = 0;

		/// Validates a single \a notification with contextual information \a args.
		virtual ValidationResult validate(const TNotification& notification, TArgs&&... args) const = 0;
	};
}}

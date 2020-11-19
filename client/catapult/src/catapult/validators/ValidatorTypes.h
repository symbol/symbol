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
#include "AggregateNotificationValidator.h"
#include "EntityValidator.h"
#include "FunctionalNotificationValidator.h"
#include "catapult/model/VerifiableEntity.h"
#include "catapult/model/WeakEntityInfo.h"
#include "catapult/functions.h"
#include <memory>
#include <vector>

namespace catapult { namespace validators {

	struct ValidatorContext;

	template<typename... TArgs>
	class DemuxValidatorBuilderT;

	/// Validation result predicate.
	using ValidationResultPredicate = predicate<ValidationResult>;

	namespace stateless {
		template<typename TNotification>
		using NotificationValidatorT = catapult::validators::NotificationValidatorT<TNotification>;
		using NotificationValidator = NotificationValidatorT<model::Notification>;

		template<typename TNotification>
		using NotificationValidatorPointerT = std::unique_ptr<const NotificationValidatorT<TNotification>>;

		template<typename TNotification>
		using FunctionalNotificationValidatorT = catapult::validators::FunctionalNotificationValidatorT<TNotification>;

		using AggregateNotificationValidator = AggregateNotificationValidatorT<model::Notification>;
		using DemuxValidatorBuilder = DemuxValidatorBuilderT<>;
	}

	namespace stateful {
		template<typename TNotification>
		using NotificationValidatorT = catapult::validators::NotificationValidatorT<TNotification, const ValidatorContext&>;
		using NotificationValidator = NotificationValidatorT<model::Notification>;

		template<typename TNotification>
		using NotificationValidatorPointerT = std::unique_ptr<const NotificationValidatorT<TNotification>>;

		template<typename TNotification>
		using FunctionalNotificationValidatorT =
			catapult::validators::FunctionalNotificationValidatorT<TNotification, const ValidatorContext&>;

		using AggregateNotificationValidator = AggregateNotificationValidatorT<model::Notification, const ValidatorContext&>;
		using DemuxValidatorBuilder = DemuxValidatorBuilderT<const ValidatorContext&>;
	}

/// Declares a stateless validator with \a NAME for notifications of type \a NOTIFICATION_TYPE.
#define DECLARE_STATELESS_VALIDATOR(NAME, NOTIFICATION_TYPE) \
	stateless::NotificationValidatorPointerT<NOTIFICATION_TYPE> Create##NAME##Validator

/// Makes a functional stateless validator with \a NAME around \a HANDLER for notifications of type \a NOTIFICATION_TYPE.
#define MAKE_STATELESS_VALIDATOR_WITH_TYPE(NAME, NOTIFICATION_TYPE, HANDLER) \
	std::make_unique<stateless::FunctionalNotificationValidatorT<NOTIFICATION_TYPE>>(#NAME "Validator", HANDLER)

/// Makes a functional stateless validator with \a NAME around \a HANDLER.
/// \note This macro requires a validators::Notification alias.
#define MAKE_STATELESS_VALIDATOR(NAME, HANDLER) MAKE_STATELESS_VALIDATOR_WITH_TYPE(NAME, Notification, HANDLER)

/// Defines a functional stateless validator with \a NAME around \a HANDLER.
/// \note This macro requires a validators::Notification alias.
#define DEFINE_STATELESS_VALIDATOR(NAME, HANDLER) \
	DECLARE_STATELESS_VALIDATOR(NAME, validators::Notification)() { \
		return MAKE_STATELESS_VALIDATOR(NAME, HANDLER); \
	}

/// Defines a functional stateless validator with \a NAME around \a HANDLER for notifications of type \a NOTIFICATION_TYPE.
#define DEFINE_STATELESS_VALIDATOR_WITH_TYPE(NAME, NOTIFICATION_TYPE, HANDLER) \
	DECLARE_STATELESS_VALIDATOR(NAME, NOTIFICATION_TYPE)() { \
		return std::make_unique<stateless::FunctionalNotificationValidatorT<NOTIFICATION_TYPE>>(#NAME "Validator", HANDLER); \
	}

/// Declares a stateful validator with \a NAME for notifications of type \a NOTIFICATION_TYPE.
#define DECLARE_STATEFUL_VALIDATOR(NAME, NOTIFICATION_TYPE) \
	stateful::NotificationValidatorPointerT<NOTIFICATION_TYPE> Create##NAME##Validator

/// Makes a functional stateful validator with \a NAME around \a HANDLER.
/// \note This macro requires a validators::Notification alias.
#define MAKE_STATEFUL_VALIDATOR(NAME, HANDLER) \
	std::make_unique<stateful::FunctionalNotificationValidatorT<validators::Notification>>(#NAME "Validator", HANDLER)

/// Defines a functional stateful validator with \a NAME around \a HANDLER.
/// \note This macro requires a validators::Notification alias.
#define DEFINE_STATEFUL_VALIDATOR(NAME, HANDLER) \
	DECLARE_STATEFUL_VALIDATOR(NAME, validators::Notification)() { \
		return MAKE_STATEFUL_VALIDATOR(NAME, HANDLER); \
	}

/// Defines a functional stateful validator with \a NAME around \a HANDLER for notifications of type \a NOTIFICATION_TYPE.
#define DEFINE_STATEFUL_VALIDATOR_WITH_TYPE(NAME, NOTIFICATION_TYPE, HANDLER) \
	DECLARE_STATEFUL_VALIDATOR(NAME, NOTIFICATION_TYPE)() { \
		return std::make_unique<stateful::FunctionalNotificationValidatorT<NOTIFICATION_TYPE>>(#NAME "Validator", HANDLER); \
	}
}}

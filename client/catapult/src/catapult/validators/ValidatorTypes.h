#pragma once
#include "AggregateNotificationValidator.h"
#include "EntityValidator.h"
#include "FunctionalNotificationValidator.h"
#include "catapult/model/VerifiableEntity.h"
#include "catapult/model/WeakEntityInfo.h"
#include <functional>
#include <memory>
#include <vector>

namespace catapult { namespace validators {
	struct ValidatorContext;

	template<typename... TArgs>
	class AggregateEntityValidatorT;

	template<typename... TArgs>
	class DemuxValidatorBuilderT;

	/// A vector of validators.
	template<typename... TArgs>
	using ValidatorVectorT = std::vector<std::unique_ptr<const EntityValidatorT<TArgs...>>>;

	/// A vector of validation functions.
	using ValidationFunctions = std::vector<std::function<ValidationResult (const model::WeakEntityInfo&)>>;

	/// Function signature for validation policies.
	template<typename TResult>
	using ValidationPolicyFunc = std::function<TResult (const model::WeakEntityInfos&, const ValidationFunctions&)>;

	namespace stateless {
		template<typename TNotification>
		using NotificationValidatorT = catapult::validators::NotificationValidatorT<TNotification>;

		template<typename TNotification>
		using NotificationValidatorPointerT = std::unique_ptr<const NotificationValidatorT<TNotification>>;

		template<typename TNotification>
		using FunctionalNotificationValidatorT = catapult::validators::FunctionalNotificationValidatorT<TNotification>;

		using EntityValidator = EntityValidatorT<>;
		using NotificationValidator = NotificationValidatorT<model::Notification>;

		using AggregateEntityValidator = AggregateEntityValidatorT<>;
		using AggregateNotificationValidator = AggregateNotificationValidatorT<model::Notification>;
		using DemuxValidatorBuilder = DemuxValidatorBuilderT<>;
	}

	namespace stateful {
		template<typename TNotification>
		using NotificationValidatorT = catapult::validators::NotificationValidatorT<TNotification, const ValidatorContext&>;

		template<typename TNotification>
		using NotificationValidatorPointerT = std::unique_ptr<const NotificationValidatorT<TNotification>>;

		template<typename TNotification>
		using FunctionalNotificationValidatorT =
				catapult::validators::FunctionalNotificationValidatorT<TNotification, const ValidatorContext&>;

		using EntityValidator = EntityValidatorT<const ValidatorContext&>;
		using NotificationValidator = NotificationValidatorT<model::Notification>;

		using AggregateEntityValidator = AggregateEntityValidatorT<const ValidatorContext&>;
		using AggregateNotificationValidator = AggregateNotificationValidatorT<model::Notification, const ValidatorContext&>;
		using DemuxValidatorBuilder = DemuxValidatorBuilderT<const ValidatorContext&>;
	}
}}

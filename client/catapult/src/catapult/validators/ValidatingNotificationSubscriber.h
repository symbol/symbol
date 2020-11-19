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
#include "AggregateValidationResult.h"
#include "ValidatorTypes.h"
#include "catapult/model/NotificationSubscriber.h"

namespace catapult { namespace validators {

	/// Notification subscriber that validates notifications.
	class ValidatingNotificationSubscriber : public model::NotificationSubscriber {
	public:
		/// Creates a validating notification subscriber around \a validator.
		explicit ValidatingNotificationSubscriber(const stateless::NotificationValidator& validator)
				: m_validator(validator)
				, m_result(ValidationResult::Success)
		{}

	public:
		/// Gets the aggregate validation result.
		ValidationResult result() const {
			return m_result;
		}

	public:
		/// Sets a notification type exclusion \a filter.
		void setExclusionFilter(const predicate<model::NotificationType>& filter) {
			m_exclusionFilter = filter;
		}

	public:
		void notify(const model::Notification& notification) override {
			if (!IsSet(notification.Type, model::NotificationChannel::Validator))
				return;

			if (IsValidationResultFailure(m_result))
				return;

			if (m_exclusionFilter && m_exclusionFilter(notification.Type))
				return;

			auto result = m_validator.validate(notification);
			AggregateValidationResult(m_result, result);
		}

	private:
		const stateless::NotificationValidator& m_validator;
		ValidationResult m_result;
		predicate<model::NotificationType> m_exclusionFilter;
	};
}}

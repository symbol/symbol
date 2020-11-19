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
#include "catapult/utils/NamedObject.h"
#include <vector>

namespace catapult { namespace validators {

	/// Strongly typed aggregate notification validator builder.
	template<typename TNotification, typename... TArgs>
	class AggregateValidatorBuilder {
	private:
		using NotificationValidatorPointer = std::unique_ptr<const NotificationValidatorT<TNotification, TArgs...>>;
		using NotificationValidatorPointerVector = std::vector<NotificationValidatorPointer>;
		using AggregateValidatorPointer = std::unique_ptr<const AggregateNotificationValidatorT<TNotification, TArgs...>>;

	public:
		/// Adds \a pValidator to the builder and allows chaining.
		AggregateValidatorBuilder& add(NotificationValidatorPointer&& pValidator) {
			m_validators.push_back(std::move(pValidator));
			return *this;
		}

		/// Builds a strongly typed notification validator that ignores suppressed failures according to \a isSuppressedFailure.
		AggregateValidatorPointer build(const ValidationResultPredicate& isSuppressedFailure) {
			return std::make_unique<DefaultAggregateNotificationValidator>(std::move(m_validators), isSuppressedFailure);
		}

	private:
		class DefaultAggregateNotificationValidator : public AggregateNotificationValidatorT<TNotification, TArgs...> {
		public:
			DefaultAggregateNotificationValidator(
					NotificationValidatorPointerVector&& validators,
					const ValidationResultPredicate& isSuppressedFailure)
					: m_validators(std::move(validators))
					, m_isSuppressedFailure(isSuppressedFailure)
					, m_name(utils::ReduceNames(utils::ExtractNames(m_validators)))
			{}

		public:
			const std::string& name() const override {
				return m_name;
			}

			std::vector<std::string> names() const override {
				return utils::ExtractNames(m_validators);
			}

			ValidationResult validate(const TNotification& notification, TArgs&&... args) const override {
				auto aggregateResult = ValidationResult::Success;
				for (const auto& pValidator : m_validators) {
					auto result = pValidator->validate(notification, std::forward<TArgs>(args)...);

					// ignore suppressed failures
					if (m_isSuppressedFailure(result))
						continue;

					// exit on other failures
					if (IsValidationResultFailure(result))
						return result;

					AggregateValidationResult(aggregateResult, result);
				}

				return aggregateResult;
			}

		private:
			NotificationValidatorPointerVector m_validators;
			ValidationResultPredicate m_isSuppressedFailure;
			std::string m_name;
		};

	private:
		NotificationValidatorPointerVector m_validators;
	};
}}

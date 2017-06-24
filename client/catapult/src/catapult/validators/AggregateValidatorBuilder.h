#pragma once
#include "AggregateValidationResult.h"
#include "ValidatorTypes.h"
#include "catapult/utils/NamedObject.h"
#include <vector>

namespace catapult { namespace validators {

	/// A strongly typed aggregate notification validator builder.
	template<typename TNotification, typename... TArgs>
	class AggregateValidatorBuilder {
	private:
		using NotificationValidatorPointer = std::unique_ptr<const NotificationValidatorT<TNotification, TArgs...>>;
		using NotificationValidatorPointerVector = std::vector<NotificationValidatorPointer>;

	public:
		/// Adds \a pValidator to the builder and allows chaining.
		AggregateValidatorBuilder& add(NotificationValidatorPointer&& pValidator) {
			m_validators.push_back(std::move(pValidator));
			return *this;
		}

		/// Builds a strongly typed notification validator.
		std::unique_ptr<const AggregateNotificationValidatorT<TNotification, TArgs...>> build() {
			return std::make_unique<DefaultAggregateNotificationValidator>(std::move(m_validators));
		}

	private:
		class DefaultAggregateNotificationValidator : public AggregateNotificationValidatorT<TNotification, TArgs...> {
		public:
			explicit DefaultAggregateNotificationValidator(NotificationValidatorPointerVector&& validators)
					: m_validators(std::move(validators))
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
				for (auto iter = m_validators.cbegin(); m_validators.cend() != iter; ++iter) {
					auto result = (*iter)->validate(notification, std::forward<TArgs>(args)...);
					if (IsValidationResultFailure(result))
						return result;

					AggregateValidationResult(aggregateResult, result);
				}

				return aggregateResult;
			}

		private:
			NotificationValidatorPointerVector m_validators;
			std::string m_name;
		};

	private:
		NotificationValidatorPointerVector m_validators;
	};
}}

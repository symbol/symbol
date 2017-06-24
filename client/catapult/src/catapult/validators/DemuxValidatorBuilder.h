#pragma once
#include "AggregateValidatorBuilder.h"
#include "ValidatorTypes.h"
#include <functional>
#include <vector>

namespace catapult { namespace validators {

	/// A demultiplexing validator builder.
	template<typename... TArgs>
	class DemuxValidatorBuilderT {
	private:
		template<typename TNotification>
		using NotificationValidatorPointerT = std::unique_ptr<const NotificationValidatorT<TNotification, TArgs...>>;
		using NotificationValidatorPredicate = std::function<bool (const model::Notification&)>;

	public:
		/// Adds a validator (\a pValidator) to the builder that is invoked only when matching notifications are processed.
		template<
				typename TNotification,
				typename X = typename std::enable_if<!std::is_same<model::Notification, TNotification>::value>::type>
		DemuxValidatorBuilderT& add(NotificationValidatorPointerT<TNotification>&& pValidator) {
			auto predicate = [type = TNotification::Notification_Type](const auto& notification) {
				return type == notification.Type;
			};
			m_builder.add(std::make_unique<ConditionalValidator<TNotification>>(std::move(pValidator), predicate));
			return *this;
		}

		/// Adds a validator (\a pValidator) to the builder that is always invoked.
		DemuxValidatorBuilderT& add(NotificationValidatorPointerT<model::Notification>&& pValidator) {
			m_builder.add(std::move(pValidator));
			return *this;
		}

		/// Builds a demultiplexing validator.
		std::unique_ptr<const AggregateNotificationValidatorT<model::Notification, TArgs...>> build() {
			return m_builder.build();
		}

	private:
		template<typename TNotification>
		class ConditionalValidator : public NotificationValidatorT<model::Notification, TArgs...> {
		public:
			ConditionalValidator(
					NotificationValidatorPointerT<TNotification>&& pValidator,
					const NotificationValidatorPredicate& predicate)
					: m_pValidator(std::move(pValidator))
					, m_predicate(predicate)
			{}

		public:
			const std::string& name() const override {
				return m_pValidator->name();
			}

			ValidationResult validate(const model::Notification& notification, TArgs&&... args) const override {
				if (!m_predicate(notification))
					return ValidationResult::Success;

				return m_pValidator->validate(static_cast<const TNotification&>(notification), std::forward<TArgs>(args)...);
			}

		private:
			NotificationValidatorPointerT<TNotification> m_pValidator;
			NotificationValidatorPredicate m_predicate;
		};

	private:
		AggregateValidatorBuilder<model::Notification, TArgs...> m_builder;
	};
}}

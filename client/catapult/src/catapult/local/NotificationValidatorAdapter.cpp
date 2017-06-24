#include "NotificationValidatorAdapter.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransactionPlugin.h"
#include "catapult/validators/AggregateValidationResult.h"

using namespace catapult::validators;

namespace catapult { namespace local {

	namespace {
		class ValidatingNotificationSubscriber : public model::NotificationSubscriber {
		public:
			explicit ValidatingNotificationSubscriber(const stateless::NotificationValidator& validator)
					: m_validator(validator)
					, m_result(ValidationResult::Success)
			{}

		public:
			ValidationResult result() const {
				return m_result;
			}

		public:
			void notify(const model::Notification& notification) override {
				if (!IsSet(notification.Type, model::NotificationChannel::Validator))
					return;

				if (IsValidationResultFailure(m_result))
					return;

				auto result = m_validator.validate(notification);
				AggregateValidationResult(m_result, result);
			}

		private:
			const stateless::NotificationValidator& m_validator;
			ValidationResult m_result;
		};
	}

	NotificationValidatorAdapter::NotificationValidatorAdapter(
			const model::TransactionRegistry& transactionRegistry,
			NotificationValidatorPointer&& pValidator)
			: m_pValidator(std::move(pValidator))
			, m_pPub(CreateNotificationPublisher(transactionRegistry))
	{}

	const std::string& NotificationValidatorAdapter::name() const {
		return m_pValidator->name();
	}

	ValidationResult NotificationValidatorAdapter::validate(const model::WeakEntityInfo& entityInfo) const {
		ValidatingNotificationSubscriber sub(*m_pValidator);
		m_pPub->publish(entityInfo, sub);
		return sub.result();
	}
}}

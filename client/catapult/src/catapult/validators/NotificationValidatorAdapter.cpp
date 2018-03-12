#include "NotificationValidatorAdapter.h"
#include "ValidatingNotificationSubscriber.h"
#include "catapult/model/TransactionPlugin.h"

namespace catapult { namespace validators {

	NotificationValidatorAdapter::NotificationValidatorAdapter(
			NotificationValidatorPointer&& pValidator,
			NotificationPublisherPointer&& pPublisher)
			: m_pValidator(std::move(pValidator))
			, m_pPublisher(std::move(pPublisher))
	{}

	const std::string& NotificationValidatorAdapter::name() const {
		return m_pValidator->name();
	}

	ValidationResult NotificationValidatorAdapter::validate(const model::WeakEntityInfo& entityInfo) const {
		ValidatingNotificationSubscriber sub(*m_pValidator);
		m_pPublisher->publish(entityInfo, sub);
		return sub.result();
	}
}}

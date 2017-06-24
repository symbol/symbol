#include "ProcessingNotificationSubscriber.h"
#include "catapult/validators/AggregateValidationResult.h"

namespace catapult { namespace chain {

	ProcessingNotificationSubscriber::ProcessingNotificationSubscriber(
			const validators::stateful::NotificationValidator& validator,
			const validators::ValidatorContext& validatorContext,
			const observers::NotificationObserver& observer,
			const observers::ObserverContext& observerContext)
			: m_validator(validator)
			, m_validatorContext(validatorContext)
			, m_observer(observer)
			, m_observerContext(observerContext)
			, m_aggregateResult(validators::ValidationResult::Success)
			, m_isUndoEnabled(false)
	{}

	validators::ValidationResult ProcessingNotificationSubscriber::result() const {
		return m_aggregateResult;
	}

	void ProcessingNotificationSubscriber::enableUndo() {
		m_isUndoEnabled = true;
	}

	void ProcessingNotificationSubscriber::undo() {
		if (!m_isUndoEnabled)
			CATAPULT_THROW_RUNTIME_ERROR("cannot undo because undo is not enabled");

		auto undoMode = observers::NotifyMode::Commit == m_observerContext.Mode
				? observers::NotifyMode::Rollback
				: observers::NotifyMode::Commit;
		auto undoObserverContext = observers::ObserverContext(
				m_observerContext.Cache,
				m_observerContext.State,
				m_observerContext.Height,
				undoMode);
		for (auto iter = m_notificationBuffers.crbegin(); m_notificationBuffers.crend() != iter; ++iter) {
			const auto* pNotification = reinterpret_cast<const model::Notification*>(iter->data());
			m_observer.notify(*pNotification, undoObserverContext);
		}

		m_notificationBuffers.clear();
	}

	void ProcessingNotificationSubscriber::notify(const model::Notification& notification) {
		if (notification.Size < sizeof(model::Notification))
			CATAPULT_THROW_INVALID_ARGUMENT("cannot process notification with incorrect size");

		if (!IsValidationResultSuccess(m_aggregateResult))
			return;

		validate(notification);
		if (!IsValidationResultSuccess(m_aggregateResult))
			return;

		observe(notification);
	}

	void ProcessingNotificationSubscriber::validate(const model::Notification& notification) {
		if (!IsSet(notification.Type, model::NotificationChannel::Validator))
			return;

		auto result = m_validator.validate(notification, m_validatorContext);
		AggregateValidationResult(m_aggregateResult, result);
	}

	void ProcessingNotificationSubscriber::observe(const model::Notification& notification) {
		if (!IsSet(notification.Type, model::NotificationChannel::Observer))
			return;

		m_observer.notify(notification, m_observerContext);

		if (!m_isUndoEnabled)
			return;

		// store a copy of the notification buffer
		const auto* pData = reinterpret_cast<const uint8_t*>(&notification);
		m_notificationBuffers.emplace_back(pData, pData + notification.Size);
	}
}}

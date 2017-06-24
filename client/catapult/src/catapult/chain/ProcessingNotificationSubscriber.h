#pragma once
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/observers/ObserverTypes.h"
#include "catapult/validators/ValidatorContext.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace chain {

	/// A notification subscriber that processes notifications.
	class ProcessingNotificationSubscriber : public model::NotificationSubscriber {
	public:
		/// Creates a subscriber around \a validator, \a validatorContext, \a observer, and \a observerContext.
		ProcessingNotificationSubscriber(
				const validators::stateful::NotificationValidator& validator,
				const validators::ValidatorContext& validatorContext,
				const observers::NotificationObserver& observer,
				const observers::ObserverContext& observerContext);

	public:
		/// The aggregate result of processed notifications.
		validators::ValidationResult result() const;

	public:
		/// Enables subsequent notifications to be undone.
		void enableUndo();

		/// Undoes all executions since enableUndo was first called.
		void undo();

	public:
		void notify(const model::Notification& notification) override;

	private:
		void validate(const model::Notification& notification);
		void observe(const model::Notification& notification);

	private:
		const validators::stateful::NotificationValidator& m_validator;
		const validators::ValidatorContext& m_validatorContext;
		const observers::NotificationObserver& m_observer;
		const observers::ObserverContext& m_observerContext;

		validators::ValidationResult m_aggregateResult;
		bool m_isUndoEnabled;
		std::vector<std::vector<uint8_t>> m_notificationBuffers;
	};
}}

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
#include "ProcessingUndoNotificationSubscriber.h"
#include "catapult/validators/ValidatorContext.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace chain {

	/// Notification subscriber that processes notifications.
	class ProcessingNotificationSubscriber : public model::NotificationSubscriber {
	public:
		/// Creates a subscriber around \a validator, \a validatorContext, \a observer and \a observerContext.
		ProcessingNotificationSubscriber(
				const validators::stateful::NotificationValidator& validator,
				const validators::ValidatorContext& validatorContext,
				const observers::NotificationObserver& observer,
				observers::ObserverContext& observerContext);

	public:
		/// Gets the aggregate result of processed notifications.
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
		observers::ObserverContext& m_observerContext;

		ProcessingUndoNotificationSubscriber m_undoNotificationSubscriber;
		validators::ValidationResult m_aggregateResult;
		bool m_isUndoEnabled;
	};
}}

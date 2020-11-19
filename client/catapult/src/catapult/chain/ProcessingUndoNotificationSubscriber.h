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
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/observers/ObserverTypes.h"

namespace catapult { namespace chain {

	/// Notification subscriber that captures notifications and allows them to be undone.
	class ProcessingUndoNotificationSubscriber : public model::NotificationSubscriber {
	public:
		/// Creates a subscriber around \a observer and \a observerContext.
		ProcessingUndoNotificationSubscriber(const observers::NotificationObserver& observer, observers::ObserverContext& observerContext);

	public:
		/// Undoes all executions.
		void undo();

	public:
		void notify(const model::Notification& notification) override;

	private:
		void observe(const model::Notification& notification);

	private:
		const observers::NotificationObserver& m_observer;
		observers::ObserverContext& m_observerContext;

		std::vector<std::vector<uint8_t>> m_notificationBuffers;
	};
}}

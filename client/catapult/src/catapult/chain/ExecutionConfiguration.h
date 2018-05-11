/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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
#include "catapult/model/NetworkInfo.h"
#include "catapult/model/NotificationPublisher.h"
#include "catapult/observers/ObserverTypes.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace chain {

	/// Configuration for executing entities.
	struct ExecutionConfiguration {
	private:
		using ObserverPointer = std::shared_ptr<const observers::AggregateNotificationObserver>;
		using ValidatorPointer = std::shared_ptr<const validators::stateful::AggregateNotificationValidator>;
		using PublisherPointer = std::shared_ptr<const model::NotificationPublisher>;

	public:
		/// Network info.
		model::NetworkInfo Network;

		/// Observer.
		ObserverPointer pObserver;

		/// Stateful validator.
		ValidatorPointer pValidator;

		/// Notification publisher.
		PublisherPointer pNotificationPublisher;
	};
}}

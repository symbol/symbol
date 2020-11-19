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

	void NotificationValidatorAdapter::setExclusionFilter(const predicate<model::NotificationType>& filter) {
		m_exclusionFilter = filter;
	}

	const std::string& NotificationValidatorAdapter::name() const {
		return m_pValidator->name();
	}

	ValidationResult NotificationValidatorAdapter::validate(const model::WeakEntityInfo& entityInfo) const {
		ValidatingNotificationSubscriber sub(*m_pValidator);
		sub.setExclusionFilter(m_exclusionFilter);

		m_pPublisher->publish(entityInfo, sub);
		return sub.result();
	}
}}

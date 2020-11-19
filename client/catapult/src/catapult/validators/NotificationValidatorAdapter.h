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
#include "ValidatorTypes.h"
#include "catapult/model/NotificationPublisher.h"

namespace catapult { namespace model { class TransactionRegistry; } }

namespace catapult { namespace validators {

	/// Stateless notification validator to entity validator adapter.
	class NotificationValidatorAdapter : public StatelessEntityValidator {
	private:
		using NotificationValidatorPointer = std::unique_ptr<const stateless::NotificationValidator>;
		using NotificationPublisherPointer = std::unique_ptr<const model::NotificationPublisher>;

	public:
		/// Creates a new adapter around \a pValidator and \a pPublisher.
		NotificationValidatorAdapter(NotificationValidatorPointer&& pValidator, NotificationPublisherPointer&& pPublisher);

	public:
		/// Sets a notification type exclusion \a filter.
		void setExclusionFilter(const predicate<model::NotificationType>& filter);

	public:
		const std::string& name() const override;

		ValidationResult validate(const model::WeakEntityInfo& entityInfo) const override;

	private:
		NotificationValidatorPointer m_pValidator;
		NotificationPublisherPointer m_pPublisher;
		predicate<model::NotificationType> m_exclusionFilter;
	};
}}

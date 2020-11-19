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
#include "NotificationValidator.h"
#include <functional>

namespace catapult { namespace validators {

	/// Notification validator that wraps a function.
	template<typename TNotification, typename... TArgs>
	class FunctionalNotificationValidatorT : public NotificationValidatorT<TNotification, TArgs...> {
	private:
		using FunctionType = std::function<ValidationResult (const TNotification&, TArgs&&...)>;

	public:
		/// Creates a functional notification validator around \a func with \a name.
		FunctionalNotificationValidatorT(const std::string& name, const FunctionType& func)
				: m_name(name)
				, m_func(func)
		{}

	public:
		const std::string& name() const override {
			return m_name;
		}

		ValidationResult validate(const TNotification& notification, TArgs&&... args) const override {
			return m_func(notification, std::forward<TArgs>(args)...);
		}

	private:
		std::string m_name;
		FunctionType m_func;
	};
}}

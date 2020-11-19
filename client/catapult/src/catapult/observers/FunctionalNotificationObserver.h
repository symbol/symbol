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
#include "NotificationObserver.h"
#include "catapult/functions.h"

namespace catapult { namespace observers {

	/// Notification observer implementation that wraps a function.
	template<typename TNotification>
	class FunctionalNotificationObserverT : public NotificationObserverT<TNotification> {
	private:
		using FunctionType = consumer<const TNotification&, ObserverContext&>;

	public:
		/// Creates a functional notification observer around \a func with \a name.
		FunctionalNotificationObserverT(const std::string& name, const FunctionType& func)
				: m_name(name)
				, m_func(func)
		{}

	public:
		const std::string& name() const override {
			return m_name;
		}

		void notify(const TNotification& notification, ObserverContext& context) const override {
			return m_func(notification, context);
		}

	private:
		std::string m_name;
		FunctionType m_func;
	};
}}

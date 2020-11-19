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
#include "catapult/model/NotificationPublisher.h"

namespace catapult { namespace mocks {

	/// Mock notification publisher that counts the number of publish calls.
	class MockNotificationPublisher : public model::NotificationPublisher {
	public:
		/// Creates a mock notification publisher.
		MockNotificationPublisher() : m_numPublishCalls(0)
		{}

	public:
		/// Gets the number of publish calls.
		size_t numPublishCalls() const {
			return m_numPublishCalls;
		}

	public:
		void publish(const model::WeakEntityInfo&, model::NotificationSubscriber&) const override {
			++m_numPublishCalls;
		}

	private:
		mutable size_t m_numPublishCalls;
	};
}}

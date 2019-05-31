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
#include "MockNotificationObserver.h"
#include <vector>

namespace catapult { namespace mocks {

	/// Mock notification observer that captures block heights.
	class MockBlockHeightCapturingNotificationObserver : public mocks::MockNotificationObserver {
	public:
		/// Creates a mock observer around \a blockHeights.
		explicit MockBlockHeightCapturingNotificationObserver(std::vector<Height>& blockHeights)
				: MockNotificationObserverT("MockBlockHeightCapturingNotificationObserver")
				, m_blockHeights(blockHeights)
		{}

	public:
		void notify(const model::Notification& notification, observers::ObserverContext& context) const override {
			MockNotificationObserverT::notify(notification, context);

			// collect heights only when a block is processed
			if (model::Core_Block_Notification == notification.Type)
				m_blockHeights.push_back(context.Height);
		}

	private:
		std::vector<Height>& m_blockHeights;
	};
}}

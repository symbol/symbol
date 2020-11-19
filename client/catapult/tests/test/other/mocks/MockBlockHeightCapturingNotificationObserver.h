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
#include "MockNotificationObserver.h"
#include <vector>

namespace catapult { namespace mocks {

	/// Mock notification observer that captures block heights.
	class MockBlockHeightCapturingNotificationObserver : public mocks::MockNotificationObserver {
	public:
		/// Creates a mock observer around \a blockHeights.
		explicit MockBlockHeightCapturingNotificationObserver(std::vector<Height>& blockHeights)
				: MockNotificationObserverT("MockBlockHeightCapturingNotificationObserver")
				, m_pBlockHeights(&blockHeights)
				, m_pBlockStates(nullptr)
		{}

		/// Creates a mock observer around \a blockHeights and \a blockStates.
		MockBlockHeightCapturingNotificationObserver(std::vector<Height>& blockHeights, std::vector<state::CatapultState>& blockStates)
				: MockNotificationObserverT("MockBlockHeightCapturingNotificationObserver")
				, m_pBlockHeights(&blockHeights)
				, m_pBlockStates(&blockStates)
		{}

	public:
		void notify(const model::Notification& notification, observers::ObserverContext& context) const override {
			MockNotificationObserverT::notify(notification, context);

			// collect heights and states only when a block is processed
			if (model::Core_Block_Notification == notification.Type) {
				m_pBlockHeights->push_back(context.Height);

				if (m_pBlockStates)
					m_pBlockStates->push_back(context.Cache.dependentState());
			}
		}

	private:
		std::vector<Height>* m_pBlockHeights;
		std::vector<state::CatapultState>* m_pBlockStates;
	};
}}

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
#include "catapult/plugins.h"

namespace catapult {
	namespace model { class ChainScore; }
	namespace subscribers { struct StateChangeInfo; }
}

namespace catapult { namespace subscribers {

	/// State change subscriber.
	class PLUGIN_API_DEPENDENCY StateChangeSubscriber {
	public:
		virtual ~StateChangeSubscriber() = default;

	public:
		/// Indicates chain score was changed to \a chainScore.
		virtual void notifyScoreChange(const model::ChainScore& chainScore) = 0;

		/// Indicates state was changed with change information in \a stateChangeInfo.
		virtual void notifyStateChange(const StateChangeInfo& stateChangeInfo) = 0;
	};
}}

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
#include "catapult/types.h"
#include <functional>
#include <memory>
#include <unordered_set>

namespace catapult { namespace model { class ChainScore; } }

namespace catapult { namespace mongo {

	/// Interface for accessing api chain score.
	class ChainScoreProvider {
	public:
		virtual ~ChainScoreProvider()
		{}

	public:
		/// Save score (\a chainScore).
		virtual void saveScore(const model::ChainScore& chainScore) = 0;

		/// Load score. If no score has been saved 0 score will be returned.
		virtual model::ChainScore loadScore() const = 0;
	};
}}

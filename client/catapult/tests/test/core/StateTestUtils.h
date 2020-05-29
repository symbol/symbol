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
#include "catapult/state/CatapultState.h"

namespace catapult { namespace test {

	/// Creates random catapult state.
	state::CatapultState CreateRandomCatapultState();

	/// Creates deterministic catapult state.
	state::CatapultState CreateDeterministicCatapultState();

	/// Asserts that catapult state \a actual is equal to \a expected with optional \a message.
	void AssertEqual(const state::CatapultState& expected, const state::CatapultState& actual, const std::string& message = "");
}}

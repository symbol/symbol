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
#include "src/model/PropertyTypes.h"
#include "src/state/PropertyDescriptor.h"

namespace catapult { namespace state { class AccountProperties; } }

namespace catapult { namespace test {

	/// Collects all available property types.
	std::vector<model::PropertyType> CollectPropertyTypes();

	/// Creates random account properties around \a operationType and \a valuesSizes.
	state::AccountProperties CreateAccountProperties(state::OperationType operationType, const std::vector<size_t>& valuesSizes);

	/// Asserts that account properties \a actual is equal to \a expected.
	void AssertEqual(const state::AccountProperties& expected, const state::AccountProperties& actual);
}}

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
#include "src/model/AccountRestrictionFlags.h"
#include "src/state/AccountRestrictionDescriptor.h"

namespace catapult { namespace state { class AccountRestrictions; } }

namespace catapult { namespace test {

	/// Collects all available account restriction flags.
	std::vector<model::AccountRestrictionFlags> CollectAccountRestrictionFlags();

	/// Creates random account restrictions around \a operationType and \a valuesSizes.
	state::AccountRestrictions CreateAccountRestrictions(
			state::AccountRestrictionOperationType operationType,
			const std::vector<size_t>& valuesSizes);

	/// Asserts that account restrictions \a actual is equal to \a expected.
	void AssertEqual(const state::AccountRestrictions& expected, const state::AccountRestrictions& actual);
}}

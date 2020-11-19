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
#include "mongo/src/mappers/MapperInclude.h"

namespace catapult {
	namespace state {
		class AccountRestriction;
		class AccountRestrictions;
	}
}

namespace catapult { namespace test {

	/// Verifies that db restriction values (\a dbRestrictionValues) are equivalent to values of model \a restriction.
	void AssertRestrictionValues(const state::AccountRestriction& restriction, const bsoncxx::document::view& dbRestrictionValues);

	/// Verifies that db account restrictions (\a dbAccountRestrictions) are equivalent to model \a restrictions.
	void AssertEqualAccountRestrictionsData(
			const state::AccountRestrictions& restrictions,
			const bsoncxx::document::view& dbAccountRestrictions);
}}

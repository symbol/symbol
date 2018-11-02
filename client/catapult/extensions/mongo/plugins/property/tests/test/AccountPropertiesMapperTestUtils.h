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
#include "mongo/src/mappers/MapperInclude.h"

namespace catapult {
	namespace state {
		class AccountProperties;
		class AccountProperty;
	}
}

namespace catapult { namespace test {

	/// Verifies that db property values (\a dbPropertyValues) are equivalent to values of model \a property.
	void AssertPropertyValues(const state::AccountProperty& property, const bsoncxx::document::view& dbPropertyValues);

	/// Verifies that db account properties (\a dbAccountProperties) are equivalent to model \a accountProperties.
	void AssertEqualAccountPropertiesData(
			const state::AccountProperties& accountProperties,
			const bsoncxx::document::view& dbAccountProperties);
}}

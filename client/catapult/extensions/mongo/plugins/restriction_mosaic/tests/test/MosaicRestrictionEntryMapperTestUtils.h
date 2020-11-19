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
#include <bsoncxx/builder/stream/document.hpp>

namespace catapult {
	namespace state {
		class MosaicAddressRestriction;
		class MosaicGlobalRestriction;
		class MosaicRestrictionEntry;
	}
}

namespace catapult { namespace test {

	/// Mosaic address restriction traits for tests.
	struct MosaicAddressRestrictionTestTraits {
		/// Creates a mosaic address restriction with \a numValues values.
		static state::MosaicAddressRestriction CreateRestriction(size_t numValues);

		/// Asserts that model \a restrictionEntry and dbmodel \a dbRestrictionEntry are equal.
		static void AssertEqualRestriction(
				const state::MosaicRestrictionEntry& restrictionEntry,
				const bsoncxx::document::view& dbRestrictionEntry);
	};

	/// Mosaic global restriction traits for tests.
	struct MosaicGlobalRestrictionTestTraits {
		/// Creates a mosaic global restriction with \a numValues values.
		static state::MosaicGlobalRestriction CreateRestriction(size_t numValues);

		/// Asserts that model \a restrictionEntry and dbmodel \a dbRestrictionEntry are equal.
		static void AssertEqualRestriction(
				const state::MosaicRestrictionEntry& restrictionEntry,
				const bsoncxx::document::view& dbRestrictionEntry);
	};
}}

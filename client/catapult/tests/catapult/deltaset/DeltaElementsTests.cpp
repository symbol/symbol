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

#include "catapult/deltaset/DeltaElements.h"
#include "tests/TestHarness.h"

namespace catapult { namespace deltaset {

#define TEST_CLASS DeltaElementsTests

	namespace {
		using Elements = DeltaElements<std::set<int>>;
	}

	TEST(TEST_CLASS, HasChangesReturnsFalseWhenThereAreNoChanges) {
		// Arrange:
		std::set<int> empty;
		Elements elements(empty, empty, empty);

		// Assert:
		EXPECT_FALSE(elements.HasChanges());
	}

	namespace {
		void AssertHasChanges(const std::set<int>& added, const std::set<int>& removed, const std::set<int>& copied) {
			// Arrange:
			Elements elements(added, removed, copied);

			// Assert:
			EXPECT_TRUE(elements.HasChanges());
		}
	}

	TEST(TEST_CLASS, HasChangesReturnsTrueWhenAnySetHasChanges) {
		AssertHasChanges({ 1 }, {}, {});
		AssertHasChanges({}, { 1 }, {});
		AssertHasChanges({}, {}, { 1 });

		AssertHasChanges({ 1 }, { 1 }, {});
		AssertHasChanges({ 1 }, {}, { 1 });
		AssertHasChanges({}, { 1 }, { 1 });
	}

	TEST(TEST_CLASS, HasChangesReturnsTrueWhenAllSetsHaveChanges) {
		AssertHasChanges({ 1 }, { 1 }, { 1 });
	}
}}

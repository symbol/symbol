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

#include "catapult/utils/NamedObject.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS NamedObjectTests

	// region NamedObjectMixin

	TEST(TEST_CLASS, CanCreateNamedObjectMixin) {
		// Arrange + Act:
		NamedObjectMixin mixin("foo");

		// Assert:
		EXPECT_EQ("foo", mixin.name());
	}

	// endregion

	// region ExtractNames

	namespace {
		using NamedObjectPointers = std::vector<std::shared_ptr<NamedObjectMixin>>;
	}

	TEST(TEST_CLASS, CanExtractNamesFromZeroObjects) {
		// Arrange:
		NamedObjectPointers objects;

		// Act:
		auto names = ExtractNames(objects);

		// Assert:
		EXPECT_TRUE(names.empty());
	}

	TEST(TEST_CLASS, CanExtractNamesFromSingleObject) {
		// Arrange:
		NamedObjectPointers objects{ std::make_shared<NamedObjectMixin>("alpha") };

		// Act:
		auto names = ExtractNames(objects);

		// Assert:
		auto expectedNames = std::vector<std::string>{ "alpha" };
		EXPECT_EQ(expectedNames, names);
	}

	TEST(TEST_CLASS, CanExtractNamesFromMultipleObjects) {
		// Arrange:
		NamedObjectPointers objects{
			std::make_shared<NamedObjectMixin>("alpha"),
			std::make_shared<NamedObjectMixin>("OMEGA"),
			std::make_shared<NamedObjectMixin>("zEtA")
		};

		// Act:
		auto names = ExtractNames(objects);

		// Assert:
		auto expectedNames = std::vector<std::string>{ "alpha", "OMEGA", "zEtA" };
		EXPECT_EQ(expectedNames, names);
	}

	// endregion

	// region ReduceNames

	TEST(TEST_CLASS, CanReduceZeroNames) {
		// Act:
		auto name = ReduceNames({});

		// Assert:
		EXPECT_EQ("{}", name);
	}

	TEST(TEST_CLASS, CanReduceSingleName) {
		// Act:
		auto name = ReduceNames({ "alpha" });

		// Assert:
		EXPECT_EQ("{ alpha }", name);
	}

	TEST(TEST_CLASS, CanReduceMultipleNames) {
		// Act:
		auto name = ReduceNames({ "alpha", "OMEGA", "zEtA" });

		// Assert:
		EXPECT_EQ("{ alpha, OMEGA, zEtA }", name);
	}

	// endregion
}}

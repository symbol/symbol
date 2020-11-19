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

#include "catapult/utils/IdentifierGroup.h"
#include "catapult/utils/Hashers.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS IdentifierGroupTests

	namespace {
		struct TestKey_tag {};
		using TestKey = BaseValue<uint64_t, TestKey_tag>;

		// Height grouped by TestKey
		using TestIdentifierGroup = IdentifierGroup<Height, TestKey, BaseValueHasher<Height>>;

		void AssertIdentifiers(const TestIdentifierGroup::Identifiers& ids, const std::vector<Height::ValueType>& expectedIds) {
			ASSERT_EQ(expectedIds.size(), ids.size());

			for (auto id : expectedIds)
				EXPECT_EQ(1u, ids.count(Height(id))) << "identifier (height) " << id;
		}
	}

	// region ctor

	TEST(TEST_CLASS, CanCreateEmptyGroup) {
		// Act:
		TestIdentifierGroup container(TestKey(123));

		// Assert:
		EXPECT_EQ(TestKey(123), container.key());
		EXPECT_TRUE(container.empty());
		EXPECT_EQ(0u, container.size());
		AssertIdentifiers(container.identifiers(), {});
	}

	// endregion

	// region add

	TEST(TEST_CLASS, CanAddSingleIdentifier) {
		// Arrange:
		TestIdentifierGroup container(TestKey(123));

		// Act:
		container.add(Height(234));

		// Assert:
		EXPECT_EQ(TestKey(123), container.key());
		EXPECT_FALSE(container.empty());
		EXPECT_EQ(1u, container.size());
		AssertIdentifiers(container.identifiers(), { 234 });
	}

	TEST(TEST_CLASS, CanAddMultipleIdentifiers) {
		// Arrange:
		TestIdentifierGroup container(TestKey(123));
		std::vector<Height::ValueType> expectedIds{ 135, 246, 357 };

		// Act:
		for (auto id : expectedIds)
			container.add(Height(id));

		// Assert:
		EXPECT_EQ(TestKey(123), container.key());
		EXPECT_FALSE(container.empty());
		EXPECT_EQ(3u, container.size());
		AssertIdentifiers(container.identifiers(), expectedIds);
	}

	// endregion

	// region remove

	namespace {
		auto CreateTestIdentifierGroup(TestKey testKey, const std::vector<Height::ValueType>& ids) {
			TestIdentifierGroup container(testKey);
			for (auto id : ids)
				container.add(Height(id));

			// Sanity:
			EXPECT_EQ(testKey, container.key());
			EXPECT_FALSE(container.empty());
			EXPECT_EQ(ids.size(), container.size());
			AssertIdentifiers(container.identifiers(), ids);
			return container;
		}
	}

	TEST(TEST_CLASS, RemoveUnknownIdentifierIsNoOp) {
		// Arrange:
		auto container = CreateTestIdentifierGroup(TestKey(123), { 234, 345, 456 });

		// Act:
		container.remove(Height(678));
		container.remove(Height(789));

		// Assert:
		EXPECT_EQ(TestKey(123), container.key());
		EXPECT_FALSE(container.empty());
		EXPECT_EQ(3u, container.size());
		AssertIdentifiers(container.identifiers(), { 234, 345, 456 });
	}

	TEST(TEST_CLASS, CanRemoveSingleIdentifier) {
		// Arrange:
		auto container = CreateTestIdentifierGroup(TestKey(123), { 234, 345, 456 });

		// Act:
		container.remove(Height(345));

		// Assert:
		EXPECT_EQ(TestKey(123), container.key());
		EXPECT_FALSE(container.empty());
		EXPECT_EQ(2u, container.size());
		AssertIdentifiers(container.identifiers(), { 234, 456 });
	}

	TEST(TEST_CLASS, CanRemoveMultipleIdentifiers) {
		// Arrange:
		auto container = CreateTestIdentifierGroup(TestKey(123), { 234, 345, 456, 567, 678 });

		// Act:
		container.remove(Height(345));
		container.remove(Height(456));
		container.remove(Height(678));

		// Assert:
		EXPECT_EQ(TestKey(123), container.key());
		EXPECT_FALSE(container.empty());
		EXPECT_EQ(2u, container.size());
		AssertIdentifiers(container.identifiers(), { 234, 567 });
	}

	TEST(TEST_CLASS, CanRemoveAllIdentifiers) {
		// Arrange:
		auto container = CreateTestIdentifierGroup(TestKey(123), { 234, 345, 456 });

		// Act:
		container.remove(Height(234));
		container.remove(Height(345));
		container.remove(Height(456));

		// Assert:
		EXPECT_EQ(TestKey(123), container.key());
		EXPECT_TRUE(container.empty());
		EXPECT_EQ(0u, container.size());
		AssertIdentifiers(container.identifiers(), {});
	}

	// endregion
}}

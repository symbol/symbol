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

#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "tests/test/cache/SimpleCache.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS ReadOnlyViewSupplierTests

	namespace {
		using SimpleReadOnlyViewSupplier = ReadOnlyViewSupplier<test::SimpleCacheView>;

		test::SimpleCacheState CreateState(size_t id) {
			test::SimpleCacheState state;
			state.Id = id;
			return state;
		}
	}

	TEST(TEST_CLASS, CanCreateViewSupplier) {
		// Act:
		auto state = CreateState(8);
		SimpleReadOnlyViewSupplier supplier(test::SimpleCacheViewMode::Iterable, state);

		// Assert:
		EXPECT_EQ(8u, supplier.id());
	}

	TEST(TEST_CLASS, CanCreateReadOnlyView) {
		// Arrange:
		auto state = CreateState(7);
		SimpleReadOnlyViewSupplier supplier(test::SimpleCacheViewMode::Iterable, state);

		// Act:
		const auto& readOnlyView = supplier.asReadOnly();

		// Assert:
		EXPECT_EQ(7u, readOnlyView.size());
	}

	TEST(TEST_CLASS, CanMoveConstructSupplier) {
		// Arrange:
		auto state = CreateState(6);
		SimpleReadOnlyViewSupplier supplier1(test::SimpleCacheViewMode::Iterable, state);
		const auto& readOnlyView1 = supplier1.asReadOnly();

		// Act:
		auto supplier2 = std::move(supplier1);
		const auto& readOnlyView2 = supplier2.asReadOnly();

		// Assert: the moved supplier is simple and should retain its original value (move has no effect on source)
		EXPECT_NE(&readOnlyView1, &readOnlyView2);
		EXPECT_EQ(6u, readOnlyView1.size());
		EXPECT_EQ(6u, readOnlyView2.size());
	}

	TEST(TEST_CLASS, ReadOnlyViewIsReused) {
		// Arrange:
		auto state = CreateState(5);
		SimpleReadOnlyViewSupplier supplier(test::SimpleCacheViewMode::Iterable, state);

		// Act:
		const auto& readOnlyView1 = supplier.asReadOnly();
		const auto& readOnlyView2 = supplier.asReadOnly();

		// Assert: the two views are the same instance
		EXPECT_EQ(&readOnlyView1, &readOnlyView2);
		EXPECT_EQ(5u, readOnlyView1.size());
		EXPECT_EQ(5u, readOnlyView2.size());
	}
}}

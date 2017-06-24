#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "tests/test/cache/SimpleCache.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

	using SimpleReadOnlyViewSupplier = ReadOnlyViewSupplier<test::SimpleCacheView>;

	TEST(ReadOnlyViewSupplierTests, CanCreateViewSupplier) {
		// Act:
		size_t id = 8;
		SimpleReadOnlyViewSupplier supplier(id);

		// Assert:
		EXPECT_EQ(8u, supplier.id());
	}

	TEST(ReadOnlyViewSupplierTests, CanCreateReadOnlyView) {
		// Arrange:
		size_t id = 7;
		SimpleReadOnlyViewSupplier supplier(id);

		// Act:
		const auto& readOnlyView = supplier.asReadOnly();

		// Assert:
		EXPECT_EQ(7u, readOnlyView.size());
	}

	TEST(ReadOnlyViewSupplierTests, CanMoveConstructSupplier) {
		// Arrange:
		size_t id = 6;
		SimpleReadOnlyViewSupplier supplier1(id);
		const auto& readOnlyView1 = supplier1.asReadOnly();

		// Act:
		auto supplier2 = std::move(supplier1);
		const auto& readOnlyView2 = supplier2.asReadOnly();

		// Assert: the moved supplier is simple and should retain its original value (move has no effect on source)
		EXPECT_NE(&readOnlyView1, &readOnlyView2);
		EXPECT_EQ(6u, readOnlyView1.size());
		EXPECT_EQ(6u, readOnlyView2.size());
	}

	TEST(ReadOnlyViewSupplierTests, ReadOnlyViewIsReused) {
		// Arrange:
		size_t id = 5;
		SimpleReadOnlyViewSupplier supplier(id);

		// Act:
		const auto& readOnlyView1 = supplier.asReadOnly();
		const auto& readOnlyView2 = supplier.asReadOnly();

		// Assert: the two views are the same instance
		EXPECT_EQ(&readOnlyView1, &readOnlyView2);
		EXPECT_EQ(5u, readOnlyView1.size());
		EXPECT_EQ(5u, readOnlyView2.size());
	}
}}

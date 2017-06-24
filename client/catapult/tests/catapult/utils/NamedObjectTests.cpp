#include "catapult/utils/NamedObject.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

	// region NamedObjectMixin

	TEST(NamedObjectTests, CanCreateNamedObjectMixin) {
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

	TEST(NamedObjectTests, CanExtractNamesFromZeroObjects) {
		// Arrange:
		NamedObjectPointers objects;

		// Act:
		auto names = ExtractNames(objects);

		// Assert:
		EXPECT_TRUE(names.empty());
	}

	TEST(NamedObjectTests, CanExtractNamesFromSingleObject) {
		// Arrange:
		NamedObjectPointers objects{ std::make_shared<NamedObjectMixin>("alpha") };

		// Act:
		auto names = ExtractNames(objects);

		// Assert:
		auto expectedNames = std::vector<std::string>{ "alpha" };
		EXPECT_EQ(expectedNames, names);
	}

	TEST(NamedObjectTests, CanExtractNamesFromMultipleObjects) {
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

	TEST(NamedObjectTests, CanReduceZeroNames) {
		// Act:
		auto name = ReduceNames({});

		// Assert:
		EXPECT_EQ("{}", name);
	}

	TEST(NamedObjectTests, CanReduceSingleName) {
		// Act:
		auto name = ReduceNames({ "alpha" });

		// Assert:
		EXPECT_EQ("{ alpha }", name);
	}

	TEST(NamedObjectTests, CanReduceMultipleNames) {
		// Act:
		auto name = ReduceNames({ "alpha", "OMEGA", "zEtA" });

		// Assert:
		EXPECT_EQ("{ alpha, OMEGA, zEtA }", name);
	}

	// endregion
}}

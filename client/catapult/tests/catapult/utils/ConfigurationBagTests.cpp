#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/TimeSpan.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS ConfigurationBagTests

	namespace {
		using FooAlphaType = uint32_t;
		constexpr FooAlphaType Default_Foo_Alpha_Value = 123;

		constexpr auto String_With_Single_Foo_Alpha_Property = R"(
			[foo]
			alpha = 123
		)";

		constexpr ConfigurationKey Foo_Alpha_Key("foo", "alpha");
		constexpr ConfigurationKey Foo_Beta_Key("foo", "beta");
		constexpr ConfigurationKey Bar_Alpha_Key("bar", "alpha");
		constexpr ConfigurationKey Bar_Beta_Key("bar", "beta");

		ConfigurationBag LoadFromString(const std::string& str) {
			// Arrange:
			std::stringstream stream;
			stream << str;

			// Act:
			return ConfigurationBag::FromStream(stream);
		}
	}

	// region construction

	namespace {
		std::unordered_set<std::string> MakeSectionsSet(std::initializer_list<std::string> values) {
			std::unordered_set<std::string> set;
			for (const auto& value : values)
				set.insert(value);

			return set;
		}
	}

	TEST(TEST_CLASS, CanLoadBagFromEmptyIniFile) {
		// Act:
		auto bag = LoadFromString("");

		// Assert:
		EXPECT_EQ(0u, bag.size());
		EXPECT_EQ(MakeSectionsSet({}), bag.sections());
	}

	TEST(TEST_CLASS, CanLoadBagFromIniFileWithSingleValue) {
		// Act:
		auto bag = LoadFromString(String_With_Single_Foo_Alpha_Property);

		// Assert:
		EXPECT_EQ(1u, bag.size());
		EXPECT_TRUE(bag.contains(Foo_Alpha_Key));
		EXPECT_EQ(MakeSectionsSet({ "foo" }), bag.sections());
	}

	TEST(TEST_CLASS, CanLoadBagFromIniFileWithMultipleValues) {
		// Act:
		auto bag = LoadFromString(R"(
			[foo]
			alpha = 123
			beta = 99
		)");

		// Assert:
		EXPECT_EQ(2u, bag.size());
		EXPECT_TRUE(bag.contains(Foo_Alpha_Key));
		EXPECT_TRUE(bag.contains(Foo_Beta_Key));
		EXPECT_EQ(MakeSectionsSet({ "foo" }), bag.sections());
	}

	TEST(TEST_CLASS, CanLoadBagFromIniFileWithMultipleValuesInMultipleSections) {
		// Act:
		auto bag = LoadFromString(R"(
			[foo]
			alpha = 123
			[bar]
			alpha = 7
			beta = 99
		)");

		// Assert:
		EXPECT_EQ(3u, bag.size());
		EXPECT_TRUE(bag.contains(Foo_Alpha_Key));
		EXPECT_TRUE(bag.contains(Bar_Alpha_Key));
		EXPECT_TRUE(bag.contains(Bar_Beta_Key));
		EXPECT_EQ(MakeSectionsSet({ "foo", "bar" }), bag.sections());
	}

	// endregion

	// region contains

	TEST(TEST_CLASS, ContainsReturnsTrueIfBothSectionAndNameMatch) {
		// Arrange:
		auto bag = LoadFromString(String_With_Single_Foo_Alpha_Property);

		// Act + Assert:
		EXPECT_TRUE(bag.contains(Foo_Alpha_Key)); // match
	}

	TEST(TEST_CLASS, ContainsReturnsFalseIfEitherSectionOrNameDoesNotMatch) {
		// Arrange:
		auto bag = LoadFromString(String_With_Single_Foo_Alpha_Property);

		// Act + Assert:
		EXPECT_FALSE(bag.contains(Foo_Beta_Key)); // different name
		EXPECT_FALSE(bag.contains(Bar_Alpha_Key)); // different section
		EXPECT_FALSE(bag.contains(Bar_Beta_Key)); // unrelated
	}

	// endregion

	// region tryGet

	TEST(TEST_CLASS, TryGetReturnsValueIfBothSectionAndNameMatch) {
		// Arrange:
		auto bag = LoadFromString(String_With_Single_Foo_Alpha_Property);

		// Act:
		FooAlphaType value = 17;
		auto hasValidValue = bag.tryGet(Foo_Alpha_Key, value);

		// Assert:
		EXPECT_TRUE(hasValidValue);
		EXPECT_EQ(Default_Foo_Alpha_Value, value);
	}

	namespace {
		template<typename T>
		void AssertTryGetFailure(const ConfigurationKey& key, const T& initialValue) {
			// Arrange:
			auto bag = LoadFromString(String_With_Single_Foo_Alpha_Property);

			// Act:
			T value = initialValue;
			auto hasValidValue = bag.tryGet(key, value);

			// Assert: the value should not be changed on failure
			EXPECT_FALSE(hasValidValue);
			EXPECT_EQ(initialValue, value);
		}
	}

	TEST(TEST_CLASS, TryGetReturnsFalseIfEitherSectionOrNameDoesNotMatch) {
		// Assert:
		AssertTryGetFailure(Foo_Beta_Key, 17u); // different name
		AssertTryGetFailure(Bar_Alpha_Key, 18u); // different section
		AssertTryGetFailure(Bar_Beta_Key, 19u); // unrelated
	}

	TEST(TEST_CLASS, TryGetThrowsIfValueCannotBeParsed) {
		// Arrange:
		auto bag = LoadFromString(String_With_Single_Foo_Alpha_Property);
		auto initialValue = TimeSpan::FromSeconds(123);

		// Act:
		auto value = initialValue;
		EXPECT_THROW(bag.tryGet(Foo_Alpha_Key, value), property_malformed_error);

		// Assert: the value should not be changed on failure
		EXPECT_EQ(initialValue, value);
	}

	// endregion

	// region get

	TEST(TEST_CLASS, GetReturnsValueIfBothSectionAndNameMatch) {
		// Arrange:
		auto bag = LoadFromString(String_With_Single_Foo_Alpha_Property);

		// Act:
		auto value = bag.get<FooAlphaType>(Foo_Alpha_Key);

		// Assert:
		EXPECT_EQ(Default_Foo_Alpha_Value, value);
	}

	namespace {
		template<typename T, typename TException>
		void AssertGetFailure(const ConfigurationKey& key) {
			// Arrange:
			auto bag = LoadFromString(String_With_Single_Foo_Alpha_Property);

			// Act + Assert:
			EXPECT_THROW(bag.get<T>(key), TException);
		}
	}

	TEST(TEST_CLASS, GetThrowsIfEitherSectionOrNameDoesNotMatch) {
		// Assert:
		using ExceptionType = property_not_found_error;
		AssertGetFailure<FooAlphaType, ExceptionType>(Foo_Beta_Key); // different name
		AssertGetFailure<FooAlphaType, ExceptionType>(Bar_Alpha_Key); // different section
		AssertGetFailure<FooAlphaType, ExceptionType>(Bar_Beta_Key); // unrelated
	}

	TEST(TEST_CLASS, GetThrowsIfIfValueCannotBeParsed) {
		// Assert:
		using ExceptionType = property_malformed_error;
		AssertGetFailure<utils::TimeSpan, ExceptionType>(Foo_Alpha_Key);
	}

	// endregion

	// region size (section) / getAll

	namespace {
		auto LoadMultiSectionBag() {
			return LoadFromString(R"(
				[foo]
				alpha = 123
				[bar]
				alpha = 7
				beta = 99
			)");
		}
	}

	TEST(TEST_CLASS, CanRetrievePropertyCountForKnownSection) {
		// Arrange:
		auto bag = LoadMultiSectionBag();

		// Act + Assert:
		EXPECT_EQ(3u, bag.size());
		EXPECT_EQ(1u, bag.size("foo"));
		EXPECT_EQ(2u, bag.size("bar"));
	}

	TEST(TEST_CLASS, CanRetrievePropertyCountForUnknownSection) {
		// Act:
		auto bag = LoadMultiSectionBag();

		// Act + Assert:
		EXPECT_EQ(3u, bag.size());
		EXPECT_EQ(0u, bag.size("baz"));
	}

	TEST(TEST_CLASS, GetAllRetrievesAllPropertiesForKnownSection) {
		// Arrange:
		auto bag = LoadMultiSectionBag();

		// Act:
		auto fooProperties = bag.getAll<std::string>("foo");
		auto barProperties = bag.getAll<std::string>("bar");

		// Act + Assert:
		ConfigurationBag::KeyValueMap<std::string> expectedFooProperties{ { "alpha", "123" } };
		ConfigurationBag::KeyValueMap<std::string> expectedBarProperties{ { "alpha", "7" }, { "beta", "99" } };
		EXPECT_EQ(expectedFooProperties, fooProperties);
		EXPECT_EQ(expectedBarProperties, barProperties);
	}

	TEST(TEST_CLASS, GetAllReturnsEmptyPropertiesForUnknownSection) {
		// Act:
		auto bag = LoadMultiSectionBag();

		// Act:
		auto properties = bag.getAll<std::string>("baz");

		// Assert:
		EXPECT_TRUE(properties.empty());
	}

	TEST(TEST_CLASS, GetAllCanParseValuesIntoStronglyTypedValues) {
		// Arrange:
		auto bag = LoadMultiSectionBag();

		// Act:
		auto properties = bag.getAll<uint32_t>("bar");

		// Act + Assert:
		ConfigurationBag::KeyValueMap<uint32_t> expectedProperties{ { "alpha", 7 }, { "beta", 99 } };
		EXPECT_EQ(expectedProperties, properties);
	}

	TEST(TEST_CLASS, GetAllThrowsIfAnyValueCannotBeParsed) {
		// Arrange:
		auto bag = LoadFromString(R"(
			[bar]
			alpha = 7
			bad = abc
			beta = 99
		)");

		// Act + Assert:
		EXPECT_THROW(bag.getAll<uint32_t>("bar"), property_malformed_error);
	}

	// endregion
}}

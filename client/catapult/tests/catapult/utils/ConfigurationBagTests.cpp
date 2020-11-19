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

	TEST(TEST_CLASS, ContainsReturnsTrueWhenBothSectionAndNameMatch) {
		// Arrange:
		auto bag = LoadFromString(String_With_Single_Foo_Alpha_Property);

		// Act + Assert:
		EXPECT_TRUE(bag.contains(Foo_Alpha_Key)); // match
	}

	TEST(TEST_CLASS, ContainsReturnsFalseWhenEitherSectionOrNameDoesNotMatch) {
		// Arrange:
		auto bag = LoadFromString(String_With_Single_Foo_Alpha_Property);

		// Act + Assert:
		EXPECT_FALSE(bag.contains(Foo_Beta_Key)); // different name
		EXPECT_FALSE(bag.contains(Bar_Alpha_Key)); // different section
		EXPECT_FALSE(bag.contains(Bar_Beta_Key)); // unrelated
	}

	// endregion

	// region tryGet

	TEST(TEST_CLASS, TryGetReturnsValueWhenBothSectionAndNameMatch) {
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

	TEST(TEST_CLASS, TryGetReturnsFalseWhenEitherSectionOrNameDoesNotMatch) {
		AssertTryGetFailure(Foo_Beta_Key, static_cast<uint32_t>(17)); // different name
		AssertTryGetFailure(Bar_Alpha_Key, static_cast<uint32_t>(18)); // different section
		AssertTryGetFailure(Bar_Beta_Key, static_cast<uint32_t>(19)); // unrelated
	}

	TEST(TEST_CLASS, TryGetThrowsWhenValueCannotBeParsed) {
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

	TEST(TEST_CLASS, GetReturnsValueWhenBothSectionAndNameMatch) {
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

	TEST(TEST_CLASS, GetThrowsWhenEitherSectionOrNameDoesNotMatch) {
		using ExceptionType = property_not_found_error;
		AssertGetFailure<FooAlphaType, ExceptionType>(Foo_Beta_Key); // different name
		AssertGetFailure<FooAlphaType, ExceptionType>(Bar_Alpha_Key); // different section
		AssertGetFailure<FooAlphaType, ExceptionType>(Bar_Beta_Key); // unrelated
	}

	TEST(TEST_CLASS, GetThrowsWhenValueCannotBeParsed) {
		using ExceptionType = property_malformed_error;
		AssertGetFailure<utils::TimeSpan, ExceptionType>(Foo_Alpha_Key);
	}

	// endregion

	// region size (section) / getAll(Ordered)

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

	namespace {
		struct GetAllUnorderedTraits {
			template<typename T>
			using KeyValueMapType = ConfigurationBag::UnorderedKeyValueMap<T>;

			template<typename T>
			static constexpr auto GetAll(const ConfigurationBag& bag, const char* section) {
				return bag.getAll<T>(section);
			}
		};

		struct GetAllOrderedTraits {
			template<typename T>
			using KeyValueMapType = ConfigurationBag::OrderedKeyValueMap<T>;

			template<typename T>
			static constexpr auto GetAll(const ConfigurationBag& bag, const char* section) {
				return bag.getAllOrdered<T>(section);
			}
		};
	}

#define GET_ALL_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Unordered) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<GetAllUnorderedTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Ordered) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<GetAllOrderedTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	GET_ALL_TRAITS_BASED_TEST(GetAllRetrievesAllPropertiesForKnownSection) {
		// Arrange:
		auto bag = LoadMultiSectionBag();

		// Act:
		auto fooProperties = TTraits::template GetAll<std::string>(bag, "foo");
		auto barProperties = TTraits::template GetAll<std::string>(bag, "bar");

		// Act + Assert:
		using KeyValueMap = typename TTraits::template KeyValueMapType<std::string>;
		KeyValueMap expectedFooProperties{ { "alpha", "123" } };
		KeyValueMap expectedBarProperties{ { "alpha", "7" }, { "beta", "99" } };
		EXPECT_EQ(expectedFooProperties, fooProperties);
		EXPECT_EQ(expectedBarProperties, barProperties);
	}

	GET_ALL_TRAITS_BASED_TEST(GetAllReturnsEmptyPropertiesForUnknownSection) {
		// Act:
		auto bag = LoadMultiSectionBag();

		// Act:
		auto properties = TTraits::template GetAll<std::string>(bag, "baz");

		// Assert:
		EXPECT_TRUE(properties.empty());
	}

	GET_ALL_TRAITS_BASED_TEST(GetAllCanParseValuesIntoStronglyTypedValues) {
		// Arrange:
		auto bag = LoadMultiSectionBag();

		// Act:
		auto properties = TTraits::template GetAll<uint32_t>(bag, "bar");

		// Act + Assert:
		using KeyValueMap = typename TTraits::template KeyValueMapType<uint32_t>;
		KeyValueMap expectedProperties{ { "alpha", 7 }, { "beta", 99 } };
		EXPECT_EQ(expectedProperties, properties);
	}

	GET_ALL_TRAITS_BASED_TEST(GetAllThrowsWhenAnyValueCannotBeParsed) {
		// Arrange:
		auto bag = LoadFromString(R"(
			[bar]
			alpha = 7
			bad = abc
			beta = 99
		)");

		// Act + Assert:
		EXPECT_THROW(TTraits::template GetAll<uint32_t>(bag, "bar"), property_malformed_error);
	}

	// endregion
}}

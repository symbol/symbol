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
#include "catapult/utils/ConfigurationBag.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model { struct BlockChainConfiguration; } }

namespace catapult { namespace test {

	// region value parsing

	/// Asserts that \a tryParseValueFunc successfully parses \a input into \a expectedParsedValue.
	template<typename T, typename TTryParseValueFunc>
	void AssertParse(const std::string& input, const T& expectedParsedValue, TTryParseValueFunc tryParseValueFunc) {
		// Act:
		T parsedValue;
		auto isParsed = tryParseValueFunc(input, parsedValue);

		// Assert:
		std::string message = "parsing '" + input + "'";
		EXPECT_TRUE(isParsed) << message;
		EXPECT_EQ(expectedParsedValue, parsedValue) << message;
	}

	/// Asserts that \a tryParseValueFunc fails to parse \a input and the placeholder value (initialized with
	/// \a initialValue) is unchanged.
	template<typename T, typename TTryParseValueFunc>
	void AssertFailedParse(const std::string& input, const T& initialValue, TTryParseValueFunc tryParseValueFunc) {
		// Act:
		T parsedValue = initialValue;
		auto isParsed = tryParseValueFunc(input, parsedValue);

		// Assert: the value should not be changed on failure
		std::string message = "parsing '" + input + "'";
		EXPECT_FALSE(isParsed) << message;
		EXPECT_EQ(initialValue, parsedValue) << message;
	}

	/// Asserts that \a tryParseValueFunc cannot parse mutations of \a seed and the placeholder value (initialized with
	/// \a initialValue) is unchanged.
	template<typename T, typename TTryParseValueFunc>
	void AssertEnumParseFailure(const std::string& seed, const T& initialValue, TTryParseValueFunc tryParseValueFunc) {
		// Arrange:
		auto toggleCase = [](auto ch) {
			return static_cast<char>(ch >= 'a' && ch <= 'z' ? 'A' + ch - 'a' : 'a' + ch - 'A');
		};
		auto assertFailedParse = [initialValue, tryParseValueFunc](const auto& input) {
			AssertFailedParse(input, initialValue, tryParseValueFunc);
		};

		// Assert:
		assertFailedParse(""); // empty
		assertFailedParse(" " + seed); // leading space
		assertFailedParse(seed + " "); // trailing space
		assertFailedParse(toggleCase(seed[0]) + seed.substr(1)); // mismatched case
		assertFailedParse(seed.substr(0, seed.size() - 1)); // too short
		assertFailedParse(seed + "s"); // too long
		assertFailedParse("abc"); // no match
	}

	// endregion

	// region predefined tests for configuration models

	/// Asserts that a configuration supports uninitialized creation.
	template<typename TTraits>
	void AssertCanCreateUninitializedConfiguration() {
		// Act:
		auto config = TTraits::ConfigurationType::Uninitialized();

		// Assert:
		TTraits::AssertZero(config);
	}

	/// Asserts that a configuration cannot be loaded from an empty bag.
	template<typename TTraits>
	void AssertCannotLoadConfigurationFromEmptyBag() {
		// Arrange:
		auto bag = utils::ConfigurationBag(utils::ConfigurationBag::ValuesContainer());

		// Act + Assert:
		EXPECT_THROW(TTraits::ConfigurationType::LoadFromBag(bag), utils::property_not_found_error);
	}

	/// Asserts that a configuration cannot be loaded from a bag that contains a missing property.
	template<typename TTraits>
	void AssertCannotLoadConfigurationFromBagWithAnyMissingProperty() {
		// Arrange:
		for (const auto& sectionPair : TTraits::CreateProperties()) {
			const auto& section = sectionPair.first;
			if (TTraits::IsSectionOptional(section)) {
				CATAPULT_LOG(debug) << "skipping optional section " << section;
				continue;
			}

			for (const auto& namePair : sectionPair.second) {
				const auto& name = namePair.first;
				CATAPULT_LOG(debug) << "attempting to load configuration without " << section << "::" << name;

				// - copy the properties and remove the desired key
				auto propertiesCopy = TTraits::CreateProperties();
				auto& sectionProperties = propertiesCopy[section];
				auto hasNameKey = [&name](const auto& pair) { return name == pair.first; };
				sectionProperties.erase(
						std::remove_if(sectionProperties.begin(), sectionProperties.end(), hasNameKey),
						sectionProperties.end());

				// Act + Assert: the load failed
				EXPECT_THROW(TTraits::ConfigurationType::LoadFromBag(std::move(propertiesCopy)), utils::property_not_found_error);
			}
		}
	}

	/// Asserts that a configuration cannot be loaded from a bag that contains an unknown property.
	template<typename TTraits>
	void AssertCannotLoadConfigurationFromBagWithAnyUnknownProperty() {
		// Arrange:
		for (const auto& sectionPair : TTraits::CreateProperties()) {
			const auto& section = sectionPair.first;
			if (TTraits::IsSectionOptional(section)) {
				CATAPULT_LOG(debug) << "skipping optional section " << section;
				continue;
			}

			CATAPULT_LOG(debug) << "attempting to load configuration with extra property in " << section;

			// - copy the properties and add an unknown key to the desired section
			auto propertiesCopy = TTraits::CreateProperties();
			propertiesCopy[section].emplace_back("hidden", "abc");

			// Act + Assert: the load failed
			EXPECT_THROW(TTraits::ConfigurationType::LoadFromBag(std::move(propertiesCopy)), catapult_invalid_argument);
		}
	}

	/// Asserts that a configuration cannot be loaded from a bag that contains an unknown section.
	template<typename TTraits>
	void AssertCannotLoadConfigurationFromBagWithAnyUnknownSection() {
		// Arrange: add an unknown section
		auto container = TTraits::CreateProperties();
		container.insert({ "hidden", { { "foo", "1234" } } });
		auto bag = utils::ConfigurationBag(std::move(container));

		// Act + Assert:
		EXPECT_THROW(TTraits::ConfigurationType::LoadFromBag(bag), catapult_invalid_argument);
	}

	/// Asserts that a configuration loaded from a custom property bag has custom values.
	template<typename TTraits>
	void AssertCanLoadCustomConfigurationFromBag() {
		// Act:
		auto bag = utils::ConfigurationBag(TTraits::CreateProperties());
		auto config = TTraits::ConfigurationType::LoadFromBag(bag);

		// Assert:
		TTraits::AssertCustom(config);
	}

/// Adds all configuration tests to the specified test class (\a TEST_CLASS) for the configuration object
/// with the specified short name (\a SHORT_NAME).
#define DEFINE_CONFIGURATION_TESTS(TEST_CLASS, SHORT_NAME) \
	TEST(TEST_CLASS, CanCreateUninitialized##SHORT_NAME##Configuration) { \
		test::AssertCanCreateUninitializedConfiguration<SHORT_NAME##ConfigurationTraits>(); \
	} \
	TEST(TEST_CLASS, CannotLoad##SHORT_NAME##ConfigurationFromEmptyBag) { \
		test::AssertCannotLoadConfigurationFromEmptyBag<SHORT_NAME##ConfigurationTraits>(); \
	} \
	TEST(TEST_CLASS, CannotLoad##SHORT_NAME##ConfigurationFromBagWithAnyMissingProperty) { \
		test::AssertCannotLoadConfigurationFromBagWithAnyMissingProperty<SHORT_NAME##ConfigurationTraits>(); \
	} \
	TEST(TEST_CLASS, CannotLoad##SHORT_NAME##ConfigurationFromBagWithAnyUnknownProperty) { \
		test::AssertCannotLoadConfigurationFromBagWithAnyUnknownProperty<SHORT_NAME##ConfigurationTraits>(); \
	} \
	TEST(TEST_CLASS, CannotLoad##SHORT_NAME##ConfigurationFromBagWithAnyUnknownSection) { \
		test::AssertCannotLoadConfigurationFromBagWithAnyUnknownSection<SHORT_NAME##ConfigurationTraits>(); \
	} \
	TEST(TEST_CLASS, CanLoadCustom##SHORT_NAME##ConfigurationFromBag) { \
		test::AssertCanLoadCustomConfigurationFromBag<SHORT_NAME##ConfigurationTraits>(); \
	}

	// endregion
}}

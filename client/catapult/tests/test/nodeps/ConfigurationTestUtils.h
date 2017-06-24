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

		// Act:
		EXPECT_THROW(
				TTraits::ConfigurationType::LoadFromBag(bag),
				utils::property_not_found_error);
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

				// Act: copy the properties and remove the desired key
				auto copyProperties = TTraits::CreateProperties();
				copyProperties[section].erase(name);

				// Assert: the load failed
				EXPECT_THROW(
						TTraits::ConfigurationType::LoadFromBag(std::move(copyProperties)),
						utils::property_not_found_error);
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

			// Act: copy the properties and add an unknown key to the desired section
			auto copyProperties = TTraits::CreateProperties();
			copyProperties[section].emplace("hidden", "abc");

			// Assert: the load failed
			EXPECT_THROW(
					TTraits::ConfigurationType::LoadFromBag(std::move(copyProperties)),
					catapult_invalid_argument);
		}
	}

	/// Asserts that a configuration cannot be loaded from a bag that contains an unknown section.
	template<typename TTraits>
	void AssertCannotLoadConfigurationFromBagWithAnyUnknownSection() {
		// Arrange: add an unknown section
		auto container = TTraits::CreateProperties();
		container.insert({ "hidden", { { "foo", "1234" } } });
		auto bag = utils::ConfigurationBag(std::move(container));

		// Act:
		EXPECT_THROW(
				TTraits::ConfigurationType::LoadFromBag(bag),
				catapult_invalid_argument);
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

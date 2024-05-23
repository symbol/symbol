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

#include "catapult/utils/ConfigurationUtils.h"
#include "catapult/utils/HexParser.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS ConfigurationUtilsTests

	// region GetIniPropertyName

	TEST(TEST_CLASS, GetIniPropertyNameThrowsWhenCppVariableNameIsTooShort) {
		EXPECT_THROW(GetIniPropertyName(nullptr), catapult_invalid_argument);
		EXPECT_THROW(GetIniPropertyName(""), catapult_invalid_argument);
		EXPECT_THROW(GetIniPropertyName("a"), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, GetIniPropertyNameThrowsWhenCppVariableNameDoesNotStartWithLetter) {
		EXPECT_THROW(GetIniPropertyName("0abcd"), catapult_invalid_argument);
		EXPECT_THROW(GetIniPropertyName("9abcd"), catapult_invalid_argument);
		EXPECT_THROW(GetIniPropertyName("!abcd"), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, GetIniPropertyNameCanConvertValidCppVariableNames) {
		// Assert: min length
		EXPECT_EQ("aa", GetIniPropertyName("aa"));
		EXPECT_EQ("zZ", GetIniPropertyName("ZZ"));

		// - min start letter
		EXPECT_EQ("alpha", GetIniPropertyName("alpha"));
		EXPECT_EQ("alpha", GetIniPropertyName("Alpha"));

		// - max start letter
		EXPECT_EQ("zeta", GetIniPropertyName("zeta"));
		EXPECT_EQ("zeta", GetIniPropertyName("Zeta"));

		// - other
		EXPECT_EQ("fooBar", GetIniPropertyName("fooBar"));
		EXPECT_EQ("fooBar", GetIniPropertyName("FooBar"));
		EXPECT_EQ("invalid IDENTIFIER 1234!", GetIniPropertyName("Invalid IDENTIFIER 1234!"));
	}

	// endregion

	// region LoadIniProperty

	TEST(TEST_CLASS, LoadIniPropertyThrowsWhenCppVariableNameIsInvalid) {
		// Arrange:
		auto bag = ConfigurationBag({ { "foo", { { "0baz", "1234" } } } });

		// Act + Assert:
		uint32_t value;
		EXPECT_THROW(LoadIniProperty(bag, "foo", "0baz", value), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, LoadIniPropertyThrowsWhenBagDoesNotContainKey) {
		// Arrange:
		auto bag = ConfigurationBag({ { "foo", { { "baz", "1234" } } } });

		// Act + Assert:
		uint32_t value;
		EXPECT_THROW(LoadIniProperty(bag, "foo", "bar", value), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, LoadIniPropertyLoadsPropertyGivenValidKey) {
		// Arrange:
		auto bag = ConfigurationBag({ { "foo", { { "bar", "1234" } } } });

		// Act:
		uint32_t value;
		LoadIniProperty(bag, "foo", "bar", value);

		// Assert:
		EXPECT_EQ(1234u, value);
	}

	// endregion

	// region VerifyBagSizeExact

	namespace {
		ConfigurationBag CreateBagForVerifyBagSizeTests() {
			return ConfigurationBag({ { "foo", { { "bar", "1234" }, { "baz", "2345" }, { "bax", "2345" } } },
									  { "greek", { { "zeta", "55" }, { "alpha", "7" } } } });
		}
	}

	TEST(TEST_CLASS, VerifyBagSizeExactDoesNotThrowWhenBagSizeIsEqualToExpectedSize) {
		// Arrange:
		auto bag = CreateBagForVerifyBagSizeTests();

		// Act: no exceptions
		VerifyBagSizeExact(bag, 5);
	}

	TEST(TEST_CLASS, VerifyBagSizeExactThrowsWhenBagSizeIsNotEqualToExpectedSize) {
		// Arrange:
		auto bag = CreateBagForVerifyBagSizeTests();

		// Act + Assert:
		for (auto expectedSize : std::initializer_list<size_t>{ 0, 1, 4, 6, 9, 100 })
			EXPECT_THROW(VerifyBagSizeExact(bag, expectedSize), catapult_invalid_argument) << expectedSize;
	}

	// endregion

	// region ExtractSectionAsBag

	TEST(TEST_CLASS, ExtractSectionAsBagCanExtractKnownSectionAsBag) {
		// Arrange:
		auto bag = ConfigurationBag({ { "foo", { { "alpha", "123" } } }, { "bar", { { "alpha", "987" }, { "beta", "abc" } } } });

		// Act:
		auto fooBag = ExtractSectionAsBag(bag, "foo");
		auto barBag = ExtractSectionAsBag(bag, "bar");

		// Assert:
		EXPECT_EQ(1u, fooBag.size());
		EXPECT_EQ(1u, fooBag.size(""));
		EXPECT_EQ(123u, fooBag.get<uint64_t>({ "", "alpha" }));

		EXPECT_EQ(2u, barBag.size());
		EXPECT_EQ(2u, barBag.size(""));
		EXPECT_EQ(987u, barBag.get<uint64_t>({ "", "alpha" }));
		EXPECT_EQ("abc", barBag.get<std::string>({ "", "beta" }));
	}

	TEST(TEST_CLASS, ExtractSectionAsBagCanExtractUnknownSectionAsEmptyBag) {
		// Arrange:
		auto bag = ConfigurationBag({});

		// Act:
		auto fooBag = ExtractSectionAsBag(bag, "foo");

		// Assert:
		EXPECT_EQ(0u, fooBag.size());
	}

	// endregion

	// region ExtractSectionAsUnorderedSet / ExtractSectionAsOrderedVector

	namespace {
		struct UnorderedSetTraits {
			using ContainerType = std::unordered_set<std::string>;

			static constexpr auto ExtractSectionAsContainer = ExtractSectionAsUnorderedSet;
		};

		struct OrderedVectorTraits {
			using ContainerType = std::vector<std::string>;

			static constexpr auto ExtractSectionAsContainer = ExtractSectionAsOrderedVector;
		};
	}

#define CONTAINER_BASED_TEST(TEST_NAME) \
	template<typename TTraits> \
	void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_UnorderedSet) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<UnorderedSetTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_OrderedVector) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<OrderedVectorTraits>(); \
	} \
	template<typename TTraits> \
	void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	CONTAINER_BASED_TEST(ExtractSectionAsContainerCanExtractKnownSectionAsContainer) {
		// Arrange:
		using ContainerType = typename TTraits::ContainerType;
		auto bag = ConfigurationBag({ { "none", { { "zeta", "false" }, { "beta", "false" }, { "gamma", "false" } } },
									  { "some", { { "zeta", "true" }, { "beta", "false" }, { "gamma", "true" } } },
									  { "all", { { "zeta", "true" }, { "beta", "true" }, { "gamma", "true" } } } });

		// Act:
		auto noneResultPair = TTraits::ExtractSectionAsContainer(bag, "none");
		auto someResultPair = TTraits::ExtractSectionAsContainer(bag, "some");
		auto allResultPair = TTraits::ExtractSectionAsContainer(bag, "all");

		// Assert:
		EXPECT_TRUE(noneResultPair.first.empty());
		EXPECT_EQ(3u, noneResultPair.second);

		EXPECT_EQ(ContainerType({ "zeta", "gamma" }), someResultPair.first);
		EXPECT_EQ(3u, someResultPair.second);

		EXPECT_EQ(ContainerType({ "zeta", "beta", "gamma" }), allResultPair.first);
		EXPECT_EQ(3u, allResultPair.second);
	}

	CONTAINER_BASED_TEST(ExtractSectionAsContainerFailsWhenAnyValueIsNotBoolean) {
		// Arrange:
		auto bag = ConfigurationBag({ { "foo", { { "zeta", "true" }, { "beta", "1" }, { "gamma", "true" } } } });

		// Act + Assert:
		EXPECT_THROW(TTraits::ExtractSectionAsContainer(bag, "foo"), property_malformed_error);
	}

	CONTAINER_BASED_TEST(ExtractSectionAsContainerCanExtractUnknownSectionAsEmptyContainer) {
		// Arrange:
		auto bag = ConfigurationBag({});

		// Act:
		auto resultPair = TTraits::ExtractSectionAsContainer(bag, "foo");

		// Assert:
		EXPECT_TRUE(resultPair.first.empty());
		EXPECT_EQ(0u, resultPair.second);
	}

	// endregion

	// region ExtractSectionKeysAsTypedVector

	namespace {
		std::vector<std::string> GetHashStrings() {
			return { "9B9FADB394D6ABD82D3D50D7349A549929658EDB888E759709E3E5D42BABB2D6",
					 "873475716418283D4A10A62F4247C6B2B821C98573C194B8AB84E096A4EE9783",
					 "7A21C94D3860A78C49A321702F46ABD2CD3B00A59E6F87DA5EC224CEF57CC346" };
		}

		std::vector<Hash256> GetHashes(std::initializer_list<size_t> indexes) {
			auto hashStrings = GetHashStrings();

			std::vector<Hash256> hashes;
			for (auto index : indexes)
				hashes.push_back(ParseByteArray<Hash256>(hashStrings[index]));

			return hashes;
		}

		bool HashParser(const std::string& str, Hash256& hash) {
			return TryParseHexStringIntoContainer(str.data(), str.size(), hash);
		}
	}

	TEST(TEST_CLASS, ExtractSectionKeysAsTypedVectorCanExtractTypedValues) {
		// Arrange:
		auto hashStrings = GetHashStrings();
		auto bag = ConfigurationBag({ { "none", { { hashStrings[0], "false" }, { hashStrings[1], "false" }, { hashStrings[2], "false" } } },
									  { "some", { { hashStrings[0], "true" }, { hashStrings[1], "false" }, { hashStrings[2], "true" } } },
									  { "all", { { hashStrings[0], "true" }, { hashStrings[1], "true" }, { hashStrings[2], "true" } } } });

		// Act:
		auto noneResultPair = ExtractSectionKeysAsTypedVector<Hash256>(bag, "none", HashParser);
		auto someResultPair = ExtractSectionKeysAsTypedVector<Hash256>(bag, "some", HashParser);
		auto allResultPair = ExtractSectionKeysAsTypedVector<Hash256>(bag, "all", HashParser);

		// Assert:
		EXPECT_TRUE(noneResultPair.first.empty());
		EXPECT_EQ(3u, noneResultPair.second);

		EXPECT_EQ(GetHashes({ 0, 2 }), someResultPair.first);
		EXPECT_EQ(3u, someResultPair.second);

		EXPECT_EQ(GetHashes({ 0, 1, 2 }), allResultPair.first);
		EXPECT_EQ(3u, allResultPair.second);
	}

	TEST(TEST_CLASS, ExtractSectionKeysAsTypedVectorFailsWhenAnyKeyIsNotParseable) {
		// Arrange:
		auto hashStrings = GetHashStrings();
		auto bag = ConfigurationBag(
				{ { "foo", { { hashStrings[0], "true" }, { hashStrings[1] + "AA", "true" }, { hashStrings[2], "true" } } } });

		// Act + Assert:
		EXPECT_THROW(ExtractSectionKeysAsTypedVector<Hash256>(bag, "foo", HashParser), property_malformed_error);
	}

	TEST(TEST_CLASS, ExtractSectionKeysAsTypedVectorFailsWhenAnyValueIsNotBoolean) {
		// Arrange:
		auto hashStrings = GetHashStrings();
		auto bag = ConfigurationBag({ { "foo", { { hashStrings[0], "true" }, { hashStrings[1], "1" }, { hashStrings[2], "true" } } } });

		// Act + Assert:
		EXPECT_THROW(ExtractSectionKeysAsTypedVector<Hash256>(bag, "foo", HashParser), property_malformed_error);
	}

	TEST(TEST_CLASS, ExtractSectionKeysAsTypedVectorCanExtractUnknownSectionAsEmptyContainer) {
		// Arrange:
		auto bag = ConfigurationBag({});

		// Act:
		auto resultPair = ExtractSectionKeysAsTypedVector<Hash256>(bag, "foo", HashParser);

		// Assert:
		EXPECT_TRUE(resultPair.first.empty());
		EXPECT_EQ(0u, resultPair.second);
	}

	// endregion
}}

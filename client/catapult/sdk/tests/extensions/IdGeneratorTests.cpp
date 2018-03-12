#include "src/extensions/IdGenerator.h"
#include "plugins/txes/namespace/src/model/IdGenerator.h"
#include "catapult/constants.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS IdGeneratorTests

	namespace {
		template<typename TGenerator>
		void AssertDifferentNamesProduceDifferentResults(TGenerator generator) {
			// Assert:
			for (const auto name : { "bloodyrookie.alice", "nem.mex", "bloodyrookie.xem", "bloody_rookie.xem" })
				EXPECT_NE(generator("nem.xem"), generator(name)) << "nem.xem vs " << name;
		}

		template<typename TGenerator>
		void AssertNamesWithUppercaseCharactersAreRejected(TGenerator generator) {
			// Act + Assert:
			for (const auto name : { "NEM.xem", "NEM.XEM", "nem.XEM", "nEm.XeM", "NeM.xEm" }) {
				EXPECT_THROW(generator(name), catapult_invalid_argument) << "name " << name;
			}
		}

		template<typename TGenerator>
		void AssertImproperQualifiedNamesAreRejected(TGenerator generator) {
			// Act + Assert:
			for (const auto name : { ".", "..", "...", ".a", "b.", "a..b", ".a.b", "b.a." }) {
				EXPECT_THROW(generator(name), catapult_invalid_argument) << "name " << name;
			}
		}

		template<typename TGenerator>
		void AssertImproperPartNamesAreRejected(TGenerator generator) {
			// Act + Assert:
			for (const auto name : { "alpha.bet@.zeta", "a!pha.beta.zeta", "alpha.beta.ze^a" }) {
				EXPECT_THROW(generator(name), catapult_invalid_argument) << "name " << name;
			}
		}

		template<typename TGenerator>
		void AssertEmptyStringIsRejected(TGenerator generator) {
			// Act + Assert:
			EXPECT_THROW(generator(""), catapult_invalid_argument) << "empty string";
		}

#define ADD_BASIC_TESTS(PREFIX, GENERATOR) \
		TEST(TEST_CLASS, PREFIX##_DifferentNamesProduceDifferentResults) { \
			AssertDifferentNamesProduceDifferentResults(GENERATOR); \
		} \
		TEST(TEST_CLASS, PREFIX##_RejectsNamesWithUppercaseCharacters) { \
			AssertNamesWithUppercaseCharactersAreRejected(GENERATOR); \
		} \
		TEST(TEST_CLASS, PREFIX##_RejectsImproperQualifiedNames) { \
			AssertImproperQualifiedNamesAreRejected(GENERATOR); \
		} \
		TEST(TEST_CLASS, PREFIX##_RejectsImproperPartNames) { \
			AssertImproperPartNamesAreRejected(GENERATOR); \
		} \
		TEST(TEST_CLASS, PREFIX##_RejectsEmptyString) { \
			AssertEmptyStringIsRejected(GENERATOR); \
		}
	}

	// region GenerateMosaicId

	TEST(TEST_CLASS, GenerateMosaicId_GeneratesCorrectWellKnownId) {
		// Assert:
		EXPECT_EQ(Xem_Id, GenerateMosaicId("nem:xem"));
	}

	TEST(TEST_CLASS, GenerateMosaicId_SupportsMultiLevelMosaics) {
		// Arrange:
		auto expected = model::GenerateMosaicId(
				model::GenerateNamespaceId(model::GenerateNamespaceId(model::GenerateRootNamespaceId("foo"), "bar"), "baz"),
				"tokens");

		// Assert:
		EXPECT_EQ(expected, GenerateMosaicId("foo.bar.baz:tokens"));
	}

	TEST(TEST_CLASS, GenerateMosaicId_RejectsNamespaceOnlyNames) {
		// Act + Assert:
		for (const auto name : { "bloodyrookie.alice", "nem.mex", "bloodyrookie.xem", "bloody_rookie.xem" }) {
			EXPECT_THROW(GenerateMosaicId(name), catapult_invalid_argument) << "name " << name;
		}
	}

	TEST(TEST_CLASS, GenerateMosaicId_RejectsMosaicOnlyNames) {
		// Act + Assert:
		for (const auto name : { "nem", "xem", "alpha" }) {
			EXPECT_THROW(GenerateMosaicId(name), catapult_invalid_argument) << "name " << name;
		}
	}

	TEST(TEST_CLASS, GenerateMosaicId_RejectsNamesWithTooManyParts) {
		// Act + Assert:
		for (const auto name : { "a.b.c.d:e", "a.b.c.d.e:f" }) {
			EXPECT_THROW(GenerateMosaicId(name), catapult_invalid_argument) << "name " << name;
		}
	}

	TEST(TEST_CLASS, GenerateMosaicId_RejectsImproperMosaicQualifiedNames) {
		// Act + Assert:
		for (const auto name : { "a:b:c", "a::b" }) {
			EXPECT_THROW(GenerateMosaicId(name), catapult_invalid_argument) << "name " << name;
		}
	}

	namespace {
		MosaicId GenerateMosaicIdAdapter(const RawString& namespaceName) {
			// Arrange: replace the last namespace separator with a mosaic separator
			std::string namespaceAndMosaicName(namespaceName.pData, namespaceName.Size);
			auto separatorIndex = namespaceAndMosaicName.find_last_of('.');
			if (std::string::npos != separatorIndex)
				namespaceAndMosaicName[separatorIndex] = ':';

			// Act:
			return GenerateMosaicId(namespaceAndMosaicName);
		}
	}

	ADD_BASIC_TESTS(GenerateMosaicId, GenerateMosaicIdAdapter)

	// endregion

	// region GenerateNamespacePath

	TEST(TEST_CLASS, GenerateNamespacePath_GeneratesCorrectWellKnownRootPath) {
		// Act:
		auto path = GenerateNamespacePath("nem");

		// Assert:
		EXPECT_EQ(1u, path.size());
		EXPECT_EQ(Nem_Id, path[0]);
	}

	TEST(TEST_CLASS, GenerateNamespacePath_GeneratesCorrectWellKnownChildPath) {
		// Act:
		auto path = GenerateNamespacePath("nem.xem");

		// Assert:
		EXPECT_EQ(2u, path.size());
		EXPECT_EQ(Nem_Id, path[0]);
		EXPECT_EQ(NamespaceId(Xem_Id.unwrap()), path[1]);
	}

	TEST(TEST_CLASS, GenerateNamespacePath_SupportsMultiLevelNamespaces) {
		// Arrange:
		NamespacePath expected;
		expected.push_back(model::GenerateRootNamespaceId("foo"));
		expected.push_back(model::GenerateNamespaceId(expected[0], "bar"));
		expected.push_back(model::GenerateNamespaceId(expected[1], "baz"));

		// Assert:
		EXPECT_EQ(expected, GenerateNamespacePath("foo.bar.baz"));
	}

	TEST(TEST_CLASS, GenerateNamespacePath_RejectsNamesWithTooManyParts) {
		// Act + Assert:
		for (const auto name : { "a.b.c.d", "a.b.c.d.e" }) {
			EXPECT_THROW(GenerateNamespacePath(name), catapult_invalid_argument) << "name " << name;
		}
	}

	ADD_BASIC_TESTS(GenerateNamespacePath, GenerateNamespacePath)

	// endregion
}}

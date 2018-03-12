#include "src/model/IdGenerator.h"
#include "catapult/constants.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS IdGeneratorTests

	namespace {
		template<typename TGenerator>
		void AssertDifferentNamesProduceDifferentIds(TGenerator generator) {
			// Assert:
			for (const auto name : { "jeff", "bloodyrookie", "nem.xem", "nemx" })
				EXPECT_NE(generator("nem"), generator(name)) << "nem vs " << name;
		}

		template<typename TGenerator>
		void AssertDifferentlyCasedNamesProduceDifferentIds(TGenerator generator) {
			// Assert:
			for (const auto name : { "NEM", "Nem", "nEm", "NeM" })
				EXPECT_NE(generator("nem"), generator(name)) << "nem vs " << name;
		}

		template<typename TGenerator>
		void AssertDifferentParentNamespaceIdsProduceDifferentIds(TGenerator generator) {
			// Assert:
			for (auto i = 1u; i <= 5u; ++i)
				EXPECT_NE(generator(NamespaceId()), generator(NamespaceId(i))) << "ns root vs ns " << i;
		}
	}

	// region GenerateRootNamespaceId

	TEST(TEST_CLASS, GenerateRootNamespaceId_GeneratesCorrectWellKnownIds) {
		// Assert:
		EXPECT_EQ(Nem_Id, GenerateRootNamespaceId("nem"));
	}

	TEST(TEST_CLASS, GenerateRootNamespaceId_DifferentNamesProduceDifferentIds) {
		// Assert:
		AssertDifferentNamesProduceDifferentIds(GenerateRootNamespaceId);
	}

	TEST(TEST_CLASS, GenerateRootNamespaceId_NamesAreCaseSensitive) {
		// Assert:
		AssertDifferentlyCasedNamesProduceDifferentIds(GenerateRootNamespaceId);
	}

	// endregion

	// region GenerateNamespaceId

	TEST(TEST_CLASS, GenerateNamespaceId_GeneratesCorrectWellKnownIds) {
		// Assert:
		EXPECT_EQ(Nem_Id, GenerateNamespaceId(NamespaceId(), "nem"));
	}

	TEST(TEST_CLASS, GenerateNamespaceId_DifferentNamesProduceDifferentIds) {
		// Assert:
		AssertDifferentNamesProduceDifferentIds([](const auto& name) { return GenerateNamespaceId(NamespaceId(), name); });
	}

	TEST(TEST_CLASS, GenerateNamespaceId_NamesAreCaseSensitive) {
		// Assert:
		AssertDifferentlyCasedNamesProduceDifferentIds([](const auto& name) { return GenerateNamespaceId(NamespaceId(), name); });
	}

	TEST(TEST_CLASS, GenerateNamespaceId_DifferentParentNamespaceIdsProduceDifferentIds) {
		// Assert:
		AssertDifferentParentNamespaceIdsProduceDifferentIds([](const auto& nsId) { return GenerateNamespaceId(nsId, "nem"); });
	}

	TEST(TEST_CLASS, GenerateNamespaceId_CanGenerateRootNamespaceIds) {
		// Assert:
		for (const auto name : { "jeff", "bloodyrookie", "nem.xem", "nemx" })
			EXPECT_EQ(GenerateRootNamespaceId(name), GenerateNamespaceId(NamespaceId(), name)) << "ns: " << name;
	}

	// endregion

	// region GenerateMosaicId

	TEST(TEST_CLASS, GenerateMosaicId_GeneratesCorrectWellKnownIds) {
		// Assert:
		EXPECT_EQ(Xem_Id, GenerateMosaicId(Nem_Id, "xem"));
	}

	TEST(TEST_CLASS, GenerateMosaicId_DifferentNamesProduceDifferentIds) {
		// Assert:
		AssertDifferentNamesProduceDifferentIds([](const auto& name) { return GenerateMosaicId(NamespaceId(), name); });
	}

	TEST(TEST_CLASS, GenerateMosaicId_NamesAreCaseSensitive) {
		// Assert:
		AssertDifferentlyCasedNamesProduceDifferentIds([](const auto& name) { return GenerateMosaicId(NamespaceId(), name); });
	}

	TEST(TEST_CLASS, GenerateMosaicId_DifferentParentNamespaceIdsProduceDifferentIds) {
		// Assert:
		AssertDifferentParentNamespaceIdsProduceDifferentIds([](const auto& nsId) { return GenerateMosaicId(nsId, "nem"); });
	}

	// endregion
}}

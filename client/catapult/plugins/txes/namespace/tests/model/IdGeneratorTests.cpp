#include "src/model/IdGenerator.h"
#include "catapult/constants.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

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

	TEST(IdGeneratorTests, GenerateRootNamespaceId_GeneratesCorrectWellKnownIds) {
		// Assert:
		EXPECT_EQ(Nem_Id, GenerateRootNamespaceId("nem"));
	}

	TEST(IdGeneratorTests, GenerateRootNamespaceId_DifferentNamesProduceDifferentIds) {
		// Assert:
		AssertDifferentNamesProduceDifferentIds(GenerateRootNamespaceId);
	}

	TEST(IdGeneratorTests, GenerateRootNamespaceId_NamesAreCaseSensitive) {
		// Assert:
		AssertDifferentlyCasedNamesProduceDifferentIds(GenerateRootNamespaceId);
	}

	// endregion

	// region GenerateNamespaceId

	TEST(IdGeneratorTests, GenerateNamespaceId_GeneratesCorrectWellKnownIds) {
		// Assert:
		EXPECT_EQ(Nem_Id, GenerateNamespaceId(NamespaceId(), "nem"));
	}

	TEST(IdGeneratorTests, GenerateNamespaceId_DifferentNamesProduceDifferentIds) {
		// Assert:
		AssertDifferentNamesProduceDifferentIds([](const auto& name) { return GenerateNamespaceId(NamespaceId(), name); });
	}

	TEST(IdGeneratorTests, GenerateNamespaceId_NamesAreCaseSensitive) {
		// Assert:
		AssertDifferentlyCasedNamesProduceDifferentIds([](const auto& name) { return GenerateNamespaceId(NamespaceId(), name); });
	}

	TEST(IdGeneratorTests, GenerateNamespaceId_DifferentParentNamespaceIdsProduceDifferentIds) {
		// Assert:
		AssertDifferentParentNamespaceIdsProduceDifferentIds([](const auto& nsId) { return GenerateNamespaceId(nsId, "nem"); });
	}

	TEST(IdGeneratorTests, GenerateNamespaceId_CanGenerateRootNamespaceIds) {
		// Assert:
		for (const auto name : { "jeff", "bloodyrookie", "nem.xem", "nemx" })
			EXPECT_EQ(GenerateRootNamespaceId(name), GenerateNamespaceId(NamespaceId(), name)) << "ns: " << name;
	}

	// endregion

	// region GenerateMosaicId

	TEST(IdGeneratorTests, GenerateMosaicId_GeneratesCorrectWellKnownIds) {
		// Assert:
		EXPECT_EQ(Xem_Id, GenerateMosaicId(Nem_Id, "xem"));
	}

	TEST(IdGeneratorTests, GenerateMosaicId_DifferentNamesProduceDifferentIds) {
		// Assert:
		AssertDifferentNamesProduceDifferentIds([](const auto& name) { return GenerateMosaicId(NamespaceId(), name); });
	}

	TEST(IdGeneratorTests, GenerateMosaicId_NamesAreCaseSensitive) {
		// Assert:
		AssertDifferentlyCasedNamesProduceDifferentIds([](const auto& name) { return GenerateMosaicId(NamespaceId(), name); });
	}

	TEST(IdGeneratorTests, GenerateMosaicId_DifferentParentNamespaceIdsProduceDifferentIds) {
		// Assert:
		AssertDifferentParentNamespaceIdsProduceDifferentIds([](const auto& nsId) { return GenerateMosaicId(nsId, "nem"); });
	}

	// endregion
}}

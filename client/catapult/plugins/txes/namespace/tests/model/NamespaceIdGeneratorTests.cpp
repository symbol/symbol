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

#include "src/model/NamespaceIdGenerator.h"
#include "catapult/crypto/Hashes.h"
#include "tests/test/nodeps/TestConstants.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS NamespaceIdGeneratorTests

	namespace {
		template<typename TGenerator>
		void AssertDifferentNamesProduceDifferentIds(TGenerator generator) {
			// Assert:
			for (const auto name : { "jeff", "bloodyrookie", "cat.token", "catx" })
				EXPECT_NE(generator("cat"), generator(name)) << "cat vs " << name;
		}

		template<typename TGenerator>
		void AssertDifferentlyCasedNamesProduceDifferentIds(TGenerator generator) {
			// Assert:
			for (const auto name : { "CAT", "Cat", "cAt", "CaT" })
				EXPECT_NE(generator("cat"), generator(name)) << "cat vs " << name;
		}

		template<typename TGenerator>
		void AssertDifferentParentNamespaceIdsProduceDifferentIds(TGenerator generator) {
			// Assert:
			for (auto i = 1u; i <= 5; ++i)
				EXPECT_NE(generator(NamespaceId()), generator(NamespaceId(i))) << "ns root vs ns " << i;
		}
	}

	// region GenerateRootNamespaceId

	TEST(TEST_CLASS, GenerateRootNamespaceId_GeneratesCorrectWellKnownIds) {
		EXPECT_EQ(NamespaceId(test::Default_Namespace_Id), GenerateRootNamespaceId("cat"));
	}

	TEST(TEST_CLASS, GenerateRootNamespaceId_DifferentNamesProduceDifferentIds) {
		AssertDifferentNamesProduceDifferentIds(GenerateRootNamespaceId);
	}

	TEST(TEST_CLASS, GenerateRootNamespaceId_NamesAreCaseSensitive) {
		AssertDifferentlyCasedNamesProduceDifferentIds(GenerateRootNamespaceId);
	}

	// endregion

	// region GenerateNamespaceId

	TEST(TEST_CLASS, GenerateNamespaceId_GeneratesCorrectWellKnownIds) {
		EXPECT_EQ(NamespaceId(test::Default_Namespace_Id), GenerateNamespaceId(NamespaceId(), "cat"));
	}

	TEST(TEST_CLASS, GenerateNamespaceId_DifferentNamesProduceDifferentIds) {
		AssertDifferentNamesProduceDifferentIds([](const auto& name) { return GenerateNamespaceId(NamespaceId(), name); });
	}

	TEST(TEST_CLASS, GenerateNamespaceId_NamesAreCaseSensitive) {
		AssertDifferentlyCasedNamesProduceDifferentIds([](const auto& name) { return GenerateNamespaceId(NamespaceId(), name); });
	}

	TEST(TEST_CLASS, GenerateNamespaceId_DifferentParentNamespaceIdsProduceDifferentIds) {
		AssertDifferentParentNamespaceIdsProduceDifferentIds([](const auto& nsId) { return GenerateNamespaceId(nsId, "cat"); });
	}

	TEST(TEST_CLASS, GenerateNamespaceId_CanGenerateRootNamespaceIds) {
		for (const auto name : { "jeff", "bloodyrookie", "cat.token", "catx" })
			EXPECT_EQ(GenerateRootNamespaceId(name), GenerateNamespaceId(NamespaceId(), name)) << "ns: " << name;
	}

	TEST(TEST_CLASS, GenerateNamespaceId_HasHighestBitSet) {
		for (auto i = 0u; i < 1000; ++i) {
			// Act:
			auto name = test::GenerateRandomString(13);
			auto id = GenerateRootNamespaceId(name);

			// Assert:
			EXPECT_EQ(1u, id.unwrap() >> 63) << name;
		}
	}

	// endregion
}}

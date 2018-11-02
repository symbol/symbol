/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#include "src/cache/NamespaceCacheSerializers.h"
#include "tests/test/NamespaceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS NamespaceCacheSerializersTests

	namespace {
		using Serializer = NamespaceFlatMapTypesSerializer;
	}

	TEST(TEST_CLASS, FlatMapTypesSerializer_CanSerializePartialValue) {
		// Arrange:
		auto ns = state::Namespace(test::CreatePath({ 11 }));

		// Act:
		auto result = Serializer::SerializeValue(ns);

		// Assert:
		ASSERT_EQ(sizeof(uint64_t) + sizeof(NamespaceId), result.size());

		const auto* pValues = reinterpret_cast<const uint64_t*>(result.data());
		EXPECT_EQ(1u, pValues[0]);
		EXPECT_EQ(11u, pValues[1]);
	}

	TEST(TEST_CLASS, FlatMapTypesSerializer_CanSerializeFullValue) {
		// Arrange:
		auto ns = state::Namespace(test::CreatePath({ 11, 7, 21 }));

		// Act:
		auto result = Serializer::SerializeValue(ns);

		// Assert:
		ASSERT_EQ(sizeof(uint64_t) + 3 * sizeof(NamespaceId), result.size());

		const auto* pValues = reinterpret_cast<const uint64_t*>(result.data());
		EXPECT_EQ(3u, pValues[0]);
		EXPECT_EQ(11u, pValues[1]);
		EXPECT_EQ(7u, pValues[2]);
		EXPECT_EQ(21u, pValues[3]);
	}

	TEST(TEST_CLASS, FlatMapTypesSerializer_CanDeserializePartialValue) {
		// Arrange:
		auto buffer = std::vector<uint64_t>{ 1, 11 };

		// Act:
		auto ns = Serializer::DeserializeValue({ reinterpret_cast<const uint8_t*>(buffer.data()), buffer.size() * sizeof(uint64_t) });

		// Assert:
		ASSERT_EQ(1u, ns.path().size());
		EXPECT_EQ(NamespaceId(11), ns.path()[0]);
	}

	TEST(TEST_CLASS, FlatMapTypesSerializer_CanDeserializeFullValue) {
		// Arrange:
		auto buffer = std::vector<uint64_t>{ 3, 11, 7, 21 };

		// Act:
		auto ns = Serializer::DeserializeValue({ reinterpret_cast<const uint8_t*>(buffer.data()), buffer.size() * sizeof(uint64_t) });

		// Assert:
		ASSERT_EQ(3u, ns.path().size());
		EXPECT_EQ(NamespaceId(11), ns.path()[0]);
		EXPECT_EQ(NamespaceId(7), ns.path()[1]);
		EXPECT_EQ(NamespaceId(21), ns.path()[2]);
	}
}}

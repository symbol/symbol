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

#include "src/cache/NamespaceCacheSerializers.h"
#include "tests/test/NamespaceTestUtils.h"
#include "tests/test/core/BufferReader.h"
#include "tests/test/core/SerializerTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS NamespaceCacheSerializersTests

	namespace {
		using Serializer = NamespaceFlatMapTypesSerializer;
	}

	// region NamespaceFlatMapTypesSerializer Serialize

	TEST(TEST_CLASS, FlatMapTypesSerializer_CanSerializePartialValue) {
		// Arrange:
		auto ns = state::Namespace(test::CreatePath({ 11 }));

		// Act:
		auto result = Serializer::SerializeValue(ns);

		// Assert:
		ASSERT_EQ(sizeof(uint16_t) + sizeof(uint64_t) + sizeof(NamespaceId), result.size());

		test::BufferReader reader({ reinterpret_cast<const uint8_t*>(result.data()), result.size() });
		EXPECT_EQ(1u, reader.read<uint16_t>());

		EXPECT_EQ(1u, reader.read<uint64_t>());
		EXPECT_EQ(11u, reader.read<uint64_t>());
	}

	TEST(TEST_CLASS, FlatMapTypesSerializer_CanSerializeFullValue) {
		// Arrange:
		auto ns = state::Namespace(test::CreatePath({ 11, 7, 21 }));

		// Act:
		auto result = Serializer::SerializeValue(ns);

		// Assert:
		ASSERT_EQ(sizeof(uint16_t) + sizeof(uint64_t) + 3 * sizeof(NamespaceId), result.size());

		test::BufferReader reader({ reinterpret_cast<const uint8_t*>(result.data()), result.size() });
		EXPECT_EQ(1u, reader.read<uint16_t>());

		EXPECT_EQ(3u, reader.read<uint64_t>());
		EXPECT_EQ(11u, reader.read<uint64_t>());
		EXPECT_EQ(7u, reader.read<uint64_t>());
		EXPECT_EQ(21u, reader.read<uint64_t>());
	}

	// endregion

	// region NamespaceFlatMapTypesSerializer Deserialize

	TEST(TEST_CLASS, FlatMapTypesSerializer_CannotDeserializeWithInvalidVersion) {
		// Arrange:
		std::vector<uint8_t> buffer{
			2, 0, // invalid version
			1, 0, 0, 0, 0, 0, 0, 0, // path elements
			11, 0, 0, 0, 0, 0, 0, 0 // id
		};

		// Act + Assert:
		EXPECT_THROW(Serializer::DeserializeValue(buffer), catapult_runtime_error);
	}

	TEST(TEST_CLASS, FlatMapTypesSerializer_CanDeserializePartialValue) {
		// Arrange:
		std::vector<uint8_t> buffer{
			1, 0, // version
			1, 0, 0, 0, 0, 0, 0, 0, // path elements
			11, 0, 0, 0, 0, 0, 0, 0 // id
		};

		// Act:
		auto ns = Serializer::DeserializeValue(buffer);

		// Assert:
		ASSERT_EQ(1u, ns.path().size());
		EXPECT_EQ(NamespaceId(11), ns.path()[0]);
	}

	TEST(TEST_CLASS, FlatMapTypesSerializer_CanDeserializeFullValue) {
		// Arrange:
		std::vector<uint8_t> buffer{
			1, 0, // version
			3, 0, 0, 0, 0, 0, 0, 0, // path elements
			11, 0, 0, 0, 0, 0, 0, 0, // id
			7, 0, 0, 0, 0, 0, 0, 0, // id
			21, 0, 0, 0, 0, 0, 0, 0, // id
		};

		// Act:
		auto ns = Serializer::DeserializeValue(buffer);

		// Assert:
		ASSERT_EQ(3u, ns.path().size());
		EXPECT_EQ(NamespaceId(11), ns.path()[0]);
		EXPECT_EQ(NamespaceId(7), ns.path()[1]);
		EXPECT_EQ(NamespaceId(21), ns.path()[2]);
	}

	// endregion

	// region  NamespaceFlatMapTypesSerializer Roundtrip

	TEST(TEST_CLASS, FlatMapTypesSerializer_CanRoundtripPartialValue) {
		// Arrange:
		auto ns = state::Namespace(test::CreatePath({ 11 }));

		// Act:
		auto result = test::RunRoundtripStringTest<Serializer>(ns);

		// Assert:
		EXPECT_EQ(ns, result);
	}

	TEST(TEST_CLASS, FlatMapTypesSerializer_CanRoundtripFullValue) {
		// Arrange:
		auto ns = state::Namespace(test::CreatePath({ 11, 7, 21 }));

		// Act:
		auto result = test::RunRoundtripStringTest<Serializer>(ns);

		// Assert:
		EXPECT_EQ(ns, result);
	}

	// endregion
}}

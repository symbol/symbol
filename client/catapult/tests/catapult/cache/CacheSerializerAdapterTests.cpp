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

#include "catapult/cache/CacheSerializerAdapter.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS CacheSerializerAdapterTests

	namespace {
		struct MockStorage {
		public:
			using KeyType = Hash160;
			using ValueType = Hash512;

			static constexpr uint16_t State_Version = 42;

		public:
			static ValueType Load(io::InputStream& inputStream) {
				Hash512 hash;
				inputStream.read(hash);
				return hash;
			}

			static void Save(const ValueType& value, io::OutputStream& outputStream) {
				outputStream.write({ reinterpret_cast<const uint8_t*>("alpha"), 5 });
				outputStream.write(value);
			}
		};

		struct MockStorageWithBaseValueKey {
		public:
			using KeyType = Height;
			using ValueType = Hash512;
		};

		using Serializer = CacheSerializerAdapter<MockStorage>;
	}

	TEST(TEST_CLASS, SerializeValueForwardsToStorageSave) {
		// Arrange:
		auto value = test::GenerateRandomByteArray<Hash512>();

		// Act:
		auto result = Serializer::SerializeValue(value);

		// Assert:
		ASSERT_EQ(sizeof(uint16_t) + 5 + Hash512::Size, result.size());
		EXPECT_EQ(MockStorage::State_Version, reinterpret_cast<const uint16_t&>(result[0]));
		EXPECT_EQ_MEMORY("alpha", result.data() + sizeof(uint16_t), 5);
		EXPECT_EQ(value, reinterpret_cast<const Hash512&>(result[sizeof(uint16_t) + 5]));
	}

	TEST(TEST_CLASS, DeserializeValueFailsWhenThereIsNotEnoughData) {
		// Arrange:
		auto serialized = test::GenerateRandomArray<sizeof(uint16_t) + Hash512::Size - 1>();
		reinterpret_cast<uint16_t&>(serialized[0]) = MockStorage::State_Version;

		// Act + Assert:
		EXPECT_THROW(Serializer::DeserializeValue(serialized), catapult_file_io_error);
	}

	TEST(TEST_CLASS, DeserializeValueFailsWhenVersionIsIncorrect) {
		// Arrange:
		auto serialized = test::GenerateRandomArray<sizeof(uint16_t) + Hash512::Size>();
		reinterpret_cast<uint16_t&>(serialized[0]) = MockStorage::State_Version ^ 0xFFFF;

		// Act + Assert:
		EXPECT_THROW(Serializer::DeserializeValue(serialized), catapult_runtime_error);
	}

	TEST(TEST_CLASS, DeserializeValueForwardsToStorageLoad) {
		// Arrange:
		auto serialized = test::GenerateRandomArray<sizeof(uint16_t) + Hash512::Size>();
		reinterpret_cast<uint16_t&>(serialized[0]) = MockStorage::State_Version;

		// Act:
		auto result = Serializer::DeserializeValue(serialized);

		// Assert:
		EXPECT_EQ_MEMORY(serialized.data() + sizeof(uint16_t), result.data(), serialized.size() - sizeof(uint16_t));
	}
}}

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

#include "catapult/cache/CacheSerializerAdapter.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS CacheSerializerAdapterTests

	namespace {
		struct MockStorage {
		public:
			using KeyType = Hash160;
			using ValueType = Hash512;

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
		auto value = test::GenerateRandomData<Hash512_Size>();

		// Act:
		auto result = Serializer::SerializeValue(value);

		// Assert:
		ASSERT_EQ(5 + Hash512_Size, result.size());
		EXPECT_EQ_MEMORY("alpha", result.data(), 5);
		EXPECT_EQ(value, reinterpret_cast<const Hash512&>(result[5]));
	}

	TEST(TEST_CLASS, DeserializeValueFailsIfThereIsNotEnoughData) {
		// Arrange:
		auto serialized = test::GenerateRandomData<Hash512_Size - 1>();

		// Act + Assert:
		EXPECT_THROW(Serializer::DeserializeValue(serialized), catapult_file_io_error);
	}

	TEST(TEST_CLASS, DeserializeValueForwardsToStorageLoad) {
		// Arrange:
		auto serialized = test::GenerateRandomData<Hash512_Size>();

		// Act:
		auto result = Serializer::DeserializeValue(serialized);

		// Assert:
		EXPECT_EQ(serialized, result);
	}
}}

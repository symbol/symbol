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

#include "catapult/cache/IdentifierGroupSerializer.h"
#include "catapult/utils/Hashers.h"
#include "catapult/utils/IdentifierGroup.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS IdentifierGroupSerializerTests

	namespace {
		struct TimeBasedDescriptor {
		public:
			using KeyType = Timestamp;
			using ValueType = utils::IdentifierGroup<Height, Timestamp, utils::BaseValueHasher<Height>>;
		};

		using Serializer = IdentifierGroupSerializer<TimeBasedDescriptor>;

		auto GenerateRandomIdentifierGroup(size_t numIdentifiers) {
			TimeBasedDescriptor::ValueType group(test::GenerateRandomValue<Timestamp>());

			for (auto i = 0u; i < numIdentifiers; ++i)
				group.add(test::GenerateRandomValue<Height>());

			return group;
		}

		void AssertSerializedValue(const TimeBasedDescriptor::ValueType& originalGroup, const std::string& result) {
			// Assert:
			using IdentifierType = Height;
			auto expectedSize = sizeof(Timestamp) + sizeof(uint64_t) + originalGroup.size() * sizeof(IdentifierType);
			ASSERT_EQ(expectedSize, result.size());

			auto timestamp = reinterpret_cast<const Timestamp&>(result.front());
			EXPECT_EQ(originalGroup.key(), timestamp);

			auto size = *reinterpret_cast<const uint64_t*>(result.data() + sizeof(Timestamp));
			EXPECT_EQ(originalGroup.size(), size);

			const auto* pIdentifier = reinterpret_cast<const IdentifierType*>(result.data() + sizeof(Timestamp) + sizeof(uint64_t));
			const auto& expectedIdentifiers = originalGroup.identifiers();
			for (auto i = 0u; i < originalGroup.size(); ++i, ++pIdentifier)
				EXPECT_NE(expectedIdentifiers.cend(), expectedIdentifiers.find(*pIdentifier)) << "at " << i;
		}

		template<typename TContainer>
		RawBuffer ToRawBuffer(const TContainer& container) {
			return { reinterpret_cast<const uint8_t*>(container.data()), container.size() * sizeof(typename TContainer::value_type) };
		}
	}

	TEST(TEST_CLASS, CanSerializeGroupWithZeroIdentifiers) {
		// Arrange:
		auto value = GenerateRandomIdentifierGroup(0);

		// Act:
		auto result = Serializer::SerializeValue(value);

		// Assert:
		AssertSerializedValue(value, result);
	}

	TEST(TEST_CLASS, CanDeserializeGroupWithZeroIdentifiers) {
		// Arrange:
		std::vector<uint64_t> buffer{ 0x12345678'90ABCDEF, 0 };

		// Act:
		auto value = Serializer::DeserializeValue(ToRawBuffer(buffer));

		// Assert:
		EXPECT_EQ(Timestamp(buffer[0]), value.key());
		EXPECT_EQ(0u, value.size());
		EXPECT_TRUE(value.empty());
	}

	TEST(TEST_CLASS, CanSerializeValue) {
		// Arrange:
		auto value = GenerateRandomIdentifierGroup(5);

		// Act:
		auto result = Serializer::SerializeValue(value);

		// Assert:
		AssertSerializedValue(value, result);
	}

	TEST(TEST_CLASS, CanDeserializeValue) {
		// Arrange: generate data with 5 elements
		auto buffer = test::GenerateRandomDataVector<uint64_t>(1 + 1 + 5);
		buffer[1] = 5;

		// Act:
		auto value = Serializer::DeserializeValue(ToRawBuffer(buffer));

		// Assert:
		EXPECT_EQ(Timestamp(buffer[0]), value.key());
		EXPECT_EQ(5u, value.size());
		for (auto i = 2u; i < buffer.size(); ++i) {
			EXPECT_NE(value.identifiers().cend(), value.identifiers().find(Height(buffer[i])));
			value.remove(Height(buffer[i]));
		}

		EXPECT_TRUE(value.empty());
	}
}}

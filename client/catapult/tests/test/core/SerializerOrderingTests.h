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

#pragma once
#include "BufferReader.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Test suite for serializer ordering tests.
	template<typename TTraits>
	class SerializerOrderingTests {
	private:
		using KeyType = typename TTraits::KeyType;

	public:
		static void AssertSaveOrdersEntriesByKey() {
			// Arrange:
			auto entry = TTraits::CreateEntry();
			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream stream(buffer);

			// - intentionally supply unordered keys
			std::vector<KeyType> keys{ { { 123 } }, { { 34 } }, { { 57 } }, { { 12 } } };
			TTraits::AddKeys(entry, keys);

			// Act:
			TTraits::SerializerType::Save(entry, stream);

			// Assert:
			std::vector<KeyType> orderedKeys{ { { 12 } }, { { 34 } }, { { 57 } }, { { 123 } } };
			AssertKeys(orderedKeys, buffer, TTraits::GetKeyStartBufferOffset());
		}

	private:
		static void AssertKeys(const std::vector<KeyType>& expectedKeys, const RawBuffer& buffer, size_t offset) {
			ASSERT_LE(offset + sizeof(uint64_t) + expectedKeys.size() * sizeof(KeyType), buffer.Size);

			BufferReader reader(buffer);
			reader.advance(offset);
			ASSERT_EQ(expectedKeys.size(), reader.read<uint64_t>());

			for (auto i = 0u; i < expectedKeys.size(); ++i)
				EXPECT_EQ(expectedKeys[i], reader.read<KeyType>()) << "at index " << i;
		}
	};
}}

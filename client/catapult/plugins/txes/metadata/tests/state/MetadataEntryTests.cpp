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

#include "src/state/MetadataEntry.h"
#include "tests/test/MetadataTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS MetadataEntryTests

	TEST(TEST_CLASS, CanCreateMetadataEntry) {
		// Arrange:
		auto key = MetadataKey(test::GenerateRandomPartialMetadataKey());

		// Act:
		auto entry = MetadataEntry(key);

		// Assert:
		EXPECT_EQ(key.uniqueKey(), entry.key().uniqueKey());
		EXPECT_EQ(&entry.value(), &const_cast<const MetadataEntry&>(entry).value());
	}

	TEST(TEST_CLASS, CanChangeMetadataEntryValue) {
		// Arrange:
		std::vector<uint8_t> buffer{ 0x9A, 0xC7, 0x33, 0x18, 0xA7, 0xB0, 0x36 };
		auto key = MetadataKey(test::GenerateRandomPartialMetadataKey());
		auto entry = MetadataEntry(key);

		// Act:
		entry.value().update(buffer);
		const auto& valueMutable = entry.value();
		const auto& valueConst = const_cast<const MetadataEntry&>(entry).value();

		// Assert:
		ASSERT_EQ(7u, valueMutable.size());
		EXPECT_EQ_MEMORY(buffer.data(), valueMutable.data(), buffer.size());

		ASSERT_EQ(7u, valueConst.size());
		EXPECT_EQ_MEMORY(buffer.data(), valueConst.data(), buffer.size());
	}
}}

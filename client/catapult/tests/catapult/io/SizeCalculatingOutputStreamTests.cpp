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

#include "catapult/io/SizeCalculatingOutputStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace io {

#define TEST_CLASS SizeCalculatingOutputStreamTests

	TEST(TEST_CLASS, SizeOfStreamIsInitiallyZero) {
		// Act:
		SizeCalculatingOutputStream stream;

		// Assert:
		EXPECT_EQ(0u, stream.size());
	}

	TEST(TEST_CLASS, WriteIncreasesCalculatedSize) {
		// Arrange:
		SizeCalculatingOutputStream stream;
		uint64_t value = 12345;

		// Act:
		stream.write(std::vector<uint8_t>(12, 1));
		stream.write({ reinterpret_cast<uint8_t*>(&value), sizeof(uint64_t) });
		stream.write(std::array<uint8_t, 17>());

		// Assert:
		EXPECT_EQ(12u + 8 + 17, stream.size());
	}

	TEST(TEST_CLASS, FlushDoesNotChangeCalculatedSize) {
		// Arrange:
		SizeCalculatingOutputStream stream;
		stream.write(std::vector<uint8_t>(12, 1));

		// Sanity:
		EXPECT_EQ(12u, stream.size());

		// Act:
		stream.flush();

		// Assert:
		EXPECT_EQ(12u, stream.size());
	}
}}

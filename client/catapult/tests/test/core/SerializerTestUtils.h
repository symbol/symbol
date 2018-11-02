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

#pragma once
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Loads \a result from \a buffer and ensures whole buffer has been read.
	template<typename TSerializer, typename TValue>
	void RunLoadValueTest(std::vector<uint8_t>& buffer, TValue& result) {
		// Arrange:
		mocks::MockMemoryStream inputStream("", buffer);

		// Act:
		result = TSerializer::Load(inputStream);

		// Assert: whole buffer has been read
		EXPECT_EQ(buffer.size(), inputStream.position());
	}
}}

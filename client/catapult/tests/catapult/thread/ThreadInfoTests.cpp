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

#include "catapult/thread/ThreadInfo.h"
#include "tests/TestHarness.h"
#include <thread>

namespace catapult { namespace thread {

#define TEST_CLASS ThreadInfoTests

	TEST(TEST_CLASS, CanSetThreadName) {
		// Arrange:
		std::string threadName;
		std::thread([&threadName] {
			// Act:
			SetThreadName("foo BAR 123");
			threadName = GetThreadName();
		}).join();

		// Assert:
		EXPECT_EQ("foo BAR 123", threadName);
	}

	TEST(TEST_CLASS, CanSetLongTruncatedThreadName) {
		// Arrange:
		std::string threadName;
		std::thread([&threadName] {
			// Act:
			SetThreadName(std::string(2 * GetMaxThreadNameLength(), 'a'));
			threadName = GetThreadName();
		}).join();

		// Assert: the long thread name is truncated
		EXPECT_EQ(std::string(GetMaxThreadNameLength(), 'a'), threadName);
	}
}}

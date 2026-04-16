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

	TEST(TEST_CLASS, CanSetSelfThreadName) {

		std::string originalThreadName = GetThreadName();
#ifdef _WIN32
		// Assert: the default thread name is empty
		// Note: on Windows, the default thread name is empty,
		// while on Linux it is the test name.
		EXPECT_STREQ("", originalThreadName.c_str());
#endif
		// Act:
		SetThreadName("Self thread");

		// Assert:
		EXPECT_STREQ("Self thread", GetThreadName().c_str());

		// Act:
		SetThreadName("");

		// Assert: thread name is empty
		EXPECT_STREQ("", GetThreadName().c_str());

		// Restore the original thread name
		SetThreadName(originalThreadName);
		EXPECT_EQ(originalThreadName, GetThreadName());
	}

	TEST(TEST_CLASS, CanSetSpawnedThreadName) {
		// Arrange:
		std::string providedThreadName = "Spawned thread";
		std::string returnedThreadName;
		std::thread([&providedThreadName, &returnedThreadName] {
			// Act:
			SetThreadName(providedThreadName);
			returnedThreadName = GetThreadName();
		}).join();

		// Assert:
		EXPECT_EQ(providedThreadName, returnedThreadName);
	}

	TEST(TEST_CLASS, CanSetLongTruncatedThreadName) {
		// Arrange:
		std::string nameSuffix = "_SUFFIX";
		std::string providedThreadName = std::string(2 * GetMaxThreadNameLength(), 'x') + nameSuffix;
		std::string expectedThreadName = providedThreadName.substr(providedThreadName.size() - GetMaxThreadNameLength());
		std::string returnedThreadName;
		std::thread([&providedThreadName, &returnedThreadName] {
			// Act:
			SetThreadName(providedThreadName);
			returnedThreadName = GetThreadName();
		}).join();

		// Assert: the long thread name is truncated
		EXPECT_EQ(expectedThreadName, returnedThreadName);
	}

}}

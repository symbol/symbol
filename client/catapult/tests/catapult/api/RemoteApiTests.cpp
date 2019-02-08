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

#include "catapult/api/RemoteApi.h"
#include "tests/TestHarness.h"

namespace catapult { namespace api {

#define TEST_CLASS RemoteApiTests

	namespace {
		class CustomRemoteApi : public RemoteApi {
		public:
			explicit CustomRemoteApi(const Key& remotePublicKey) : RemoteApi(remotePublicKey)
			{}
		};
	}

	TEST(TEST_CLASS, CanCreateApiWithRemotePublicKey) {
		// Arrange:
		auto key = test::GenerateRandomData<Key_Size>();
		CustomRemoteApi remoteApi(key);

		// Act + Assert:
		EXPECT_EQ(key, remoteApi.remotePublicKey());
	}
}}

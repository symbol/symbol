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

#include "harvesting/src/HarvestRequest.h"
#include "harvesting/tests/test/HarvestRequestEncryptedPayload.h"
#include "tests/TestHarness.h"

namespace catapult { namespace harvesting {

#define TEST_CLASS HarvestRequestTests

	TEST(TEST_CLASS, CanGetDecryptedPayloadSize) {
		EXPECT_EQ(32u + 32, HarvestRequest::DecryptedPayloadSize());
	}

	TEST(TEST_CLASS, CanGetEncryptedPayloadSize) {
		EXPECT_EQ(32u + 16 + 12 + 32 + 32, HarvestRequest::EncryptedPayloadSize());
		EXPECT_EQ(test::HarvestRequestEncryptedPayload::Size, HarvestRequest::EncryptedPayloadSize());
	}

	TEST(TEST_CLASS, CanGetRequestIdentifierFromRequest) {
		// Arrange:
		auto expectedRequestIdentifier = test::GenerateRandomByteArray<HarvestRequestIdentifier>();

		HarvestRequest request;
		request.EncryptedPayload = RawBuffer{ expectedRequestIdentifier.data(), expectedRequestIdentifier.size() };

		// Act:
		auto requestIdentifier = GetRequestIdentifier(request);

		// Assert:
		EXPECT_EQ(expectedRequestIdentifier, requestIdentifier);
	}

	TEST(TEST_CLASS, CanRoundtripHarvestRequest) {
		// Arrange:
		auto mainAccountPublicKey = test::GenerateRandomByteArray<Key>();
		auto encryptedPayload = test::GenerateRandomVector(HarvestRequest::EncryptedPayloadSize());

		HarvestRequest originalRequest;
		originalRequest.Operation = HarvestRequestOperation::Remove;
		originalRequest.Height = Height(753);
		originalRequest.MainAccountPublicKey = mainAccountPublicKey;
		originalRequest.EncryptedPayload = RawBuffer(encryptedPayload);

		// Act: serialize
		auto buffer = SerializeHarvestRequest(originalRequest);

		// Sanity:
		ASSERT_EQ(1u + 8 + 32 + HarvestRequest::EncryptedPayloadSize(), buffer.size());

		// Act: deserialize
		auto request = DeserializeHarvestRequest(buffer);

		// Assert:
		EXPECT_EQ(HarvestRequestOperation::Remove, request.Operation);
		EXPECT_EQ(Height(753), request.Height);
		EXPECT_EQ(mainAccountPublicKey, request.MainAccountPublicKey);

		ASSERT_EQ(encryptedPayload.size(), request.EncryptedPayload.Size);
		EXPECT_NE(encryptedPayload.data(), request.EncryptedPayload.pData);
		EXPECT_EQ_MEMORY(encryptedPayload.data(), request.EncryptedPayload.pData, encryptedPayload.size());
	}
}}

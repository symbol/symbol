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

#include "catapult/ionet/PacketSocketOptions.h"
#include "tests/test/net/CertificateLocator.h"
#include "tests/TestHarness.h"
#include <boost/asio/ssl.hpp>

namespace catapult { namespace ionet {

#define TEST_CLASS PacketSocketOptionsTests

	TEST(TEST_CLASS, PacketSocketSslVerifyContext_CanCreateDefault) {
		// Act:
		PacketSocketSslVerifyContext context;

		// Assert:
		EXPECT_FALSE(context.preverified());
		EXPECT_NO_THROW(context.publicKey());
	}

	TEST(TEST_CLASS, PacketSocketSslVerifyContext_CanCreateWithArguments) {
		// Arrange:
		boost::asio::ssl::verify_context asioVerifyContext(nullptr);
		Key publicKey;

		// Act:
		PacketSocketSslVerifyContext context(true, asioVerifyContext, publicKey);

		// Assert:
		EXPECT_TRUE(context.preverified());
		EXPECT_EQ(&asioVerifyContext, &context.asioVerifyContext());
		EXPECT_EQ(&publicKey, &context.publicKey());
	}

	TEST(TEST_CLASS, PacketSocketSslVerifyContext_CanChangePublicKey) {
		// Arrange:
		boost::asio::ssl::verify_context asioVerifyContext(nullptr);
		Key publicKey;
		PacketSocketSslVerifyContext context(true, asioVerifyContext, publicKey);

		// Act:
		auto publicKey2 = test::GenerateRandomByteArray<Key>();
		context.setPublicKey(publicKey2);

		// Assert:
		EXPECT_EQ(publicKey2, publicKey);
	}

	TEST(TEST_CLASS, CanCreateSslContextSupplier) {
		// Act + Assert
		EXPECT_NO_THROW(CreateSslContextSupplier(test::GetDefaultCertificateDirectory()));
	}
}}

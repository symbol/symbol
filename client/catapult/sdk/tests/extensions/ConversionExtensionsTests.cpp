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

#include "src/extensions/ConversionExtensions.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS ConversionExtensionTests

	TEST(TEST_CLASS, CanCastMosaicIdToUnresolvedMosaicId) {
		// Arrange:
		auto mosaicId = MosaicId(248);

		// Act:
		auto unresolvedMosaicId = CastToUnresolvedMosaicId(mosaicId);

		// Assert:
		EXPECT_EQ(UnresolvedMosaicId(248), unresolvedMosaicId);
	}

	TEST(TEST_CLASS, CanCastUnresolvedMosaicIdToMosaicId) {
		// Arrange:
		auto unresolvedMosaicId = UnresolvedMosaicId(248);

		// Act:
		auto mosaicId = CastToMosaicId(unresolvedMosaicId);

		// Assert:
		EXPECT_EQ(MosaicId(248), mosaicId);
	}

	TEST(TEST_CLASS, CanCopyAddressToUnresolvedAddress) {
		// Arrange:
		auto address = test::GenerateRandomByteArray<Address>();

		// Act:
		auto unresolvedAddress = CopyToUnresolvedAddress(address);

		// Assert:
		EXPECT_EQ_MEMORY(address.data(), unresolvedAddress.data(), address.size());
	}

	TEST(TEST_CLASS, CanCopyUnresolvedAddressToAddress) {
		// Arrange:
		auto unresolvedAddress = CopyToUnresolvedAddress(test::GenerateRandomByteArray<Address>());

		// Act:
		auto address = CopyToAddress(unresolvedAddress);

		// Assert:
		EXPECT_EQ_MEMORY(unresolvedAddress.data(), address.data(), unresolvedAddress.size());
	}
}}

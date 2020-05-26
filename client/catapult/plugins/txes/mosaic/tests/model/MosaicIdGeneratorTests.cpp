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

#include "src/model/MosaicIdGenerator.h"
#include "catapult/crypto/Hashes.h"
#include "tests/test/MosaicTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS MosaicIdGeneratorTests

	// region GenerateMosaicId

	TEST(TEST_CLASS, GenerateMosaicId_GeneratesCorrectId) {
		// Arrange:
		auto buffer = std::array<uint8_t, Address::Size + sizeof(uint32_t)>();
		Hash256 zeroHash;
		crypto::Sha3_256(buffer, zeroHash);
		auto expected = reinterpret_cast<const uint64_t&>(zeroHash[0]) & 0x7FFF'FFFF'FFFF'FFFF;

		// Assert:
		EXPECT_EQ(MosaicId(expected), GenerateMosaicId(Address(), MosaicNonce()));
	}

	namespace {
		auto MutateAddress(const Address& address) {
			auto result = address;
			result[0] ^= 0xFF;
			return result;
		}
	}

	TEST(TEST_CLASS, GenerateMosaicId_DifferentAddressesProduceDifferentIds) {
		// Arrange:
		auto address1 = test::CreateRandomOwner();
		auto address2 = MutateAddress(address1);

		// Assert:
		EXPECT_NE(GenerateMosaicId(address1, MosaicNonce()), GenerateMosaicId(address2, MosaicNonce()));
	}

	TEST(TEST_CLASS, GenerateMosaicId_DifferentNoncesProduceDifferentIds) {
		// Arrange:
		auto address = test::CreateRandomOwner();
		auto nonce1 = test::GenerateRandomValue<MosaicNonce>();
		auto nonce2 = test::GenerateRandomValue<MosaicNonce>();

		// Assert: (could be equal, but rather unlikely for random nonces)
		EXPECT_NE(GenerateMosaicId(address, nonce1), GenerateMosaicId(address, nonce2));
	}

	TEST(TEST_CLASS, GenerateMosaicId_HasHighestBitCleared) {
		for (auto i = 0u; i < 1000; ++i) {
			// Act:
			auto address = test::CreateRandomOwner();
			auto nonce = test::GenerateRandomValue<MosaicNonce>();
			auto id = GenerateMosaicId(address, nonce);

			// Assert:
			EXPECT_EQ(0u, id.unwrap() >> 63) << address << " " << nonce;
		}
	}

	// endregion
}}

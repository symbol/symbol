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

#include "filespooling/src/FileTransactionStatusStorage.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace filespooling {

#define TEST_CLASS FileTransactionStatusStorageTests

	TEST(TEST_CLASS, NotifyStatusWritesToUnderlyingStream) {
		// Arrange: create output stream
		std::vector<uint8_t> buffer;
		auto pStream = std::make_unique<mocks::MockMemoryStream>(buffer);
		const auto& stream = *pStream;

		// - create status data
		auto pTransaction = test::GenerateRandomTransactionWithSize(132);
		auto hash = test::GenerateRandomByteArray<Hash256>();
		auto status = static_cast<uint32_t>(test::Random());

		// - create storage
		auto pStorage = CreateFileTransactionStatusStorage(std::move(pStream));

		// Act:
		pStorage->notifyStatus(*pTransaction, hash, status);

		// Assert:
		EXPECT_EQ(0u, stream.numFlushes());
		ASSERT_EQ(132u + Hash256::Size + sizeof(uint32_t), buffer.size());

		EXPECT_EQ(hash, reinterpret_cast<const Hash256&>(buffer[0]));
		EXPECT_EQ(status, reinterpret_cast<const uint32_t&>(buffer[Hash256::Size]));
		EXPECT_EQ(*pTransaction, reinterpret_cast<const model::Transaction&>(buffer[Hash256::Size + sizeof(uint32_t)]));
	}

	TEST(TEST_CLASS, FlushFlushesUnderlyingStream) {
		// Arrange: create output stream
		std::vector<uint8_t> buffer;
		auto pStream = std::make_unique<mocks::MockMemoryStream>(buffer);
		const auto& stream = *pStream;

		// - create storage
		auto pStorage = CreateFileTransactionStatusStorage(std::move(pStream));

		// Sanity:
		EXPECT_EQ(0u, stream.numFlushes());

		// Act:
		pStorage->flush();

		// Assert:
		EXPECT_EQ(1u, stream.numFlushes());
		EXPECT_TRUE(buffer.empty());
	}
}}

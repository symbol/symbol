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

#include "filespooling/src/FileFinalizationStorage.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace filespooling {

#define TEST_CLASS FileFinalizationStorageTests

	TEST(TEST_CLASS, NotifyFinalizedBlockWritesToUnderlyingStream) {
		// Arrange: create output stream
		std::vector<uint8_t> buffer;
		auto pStream = std::make_unique<mocks::MockMemoryStream>(buffer);
		const auto& stream = *pStream;

		// - create finalization data
		auto hash = test::GenerateRandomByteArray<Hash256>();
		auto round = model::FinalizationRound{ FinalizationEpoch(987), FinalizationPoint(456) };

		// - create storage
		auto pStorage = CreateFileFinalizationStorage(std::move(pStream));

		// Act:
		pStorage->notifyFinalizedBlock(round, Height(777), hash);

		// Assert:
		EXPECT_EQ(1u, stream.numFlushes());
		ASSERT_EQ(Hash256::Size + sizeof(model::FinalizationRound) + sizeof(Height), buffer.size());

		EXPECT_EQ(hash, reinterpret_cast<const Hash256&>(buffer[0]));
		EXPECT_EQ(round, reinterpret_cast<const model::FinalizationRound&>(buffer[Hash256::Size]));
		EXPECT_EQ(Height(777), reinterpret_cast<const Height&>(buffer[Hash256::Size + sizeof(model::FinalizationRound)]));
	}
}}

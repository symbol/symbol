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

#include "catapult/cache/SupplementalDataStorage.h"
#include "catapult/cache/SupplementalData.h"
#include "tests/test/core/StateTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS SupplementalDataStorageTests

	namespace {
		constexpr auto Data_Size = 6 * sizeof(uint64_t) + sizeof(uint32_t);
	}

	TEST(TEST_CLASS, CanSaveData) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream(buffer);

		// - create random data
		SupplementalData data;
		data.ChainScore = model::ChainScore(test::Random(), test::Random());
		data.State = test::CreateRandomCatapultState();
		auto chainHeight = test::GenerateRandomValue<Height>();

		// Act:
		SaveSupplementalData(data, chainHeight, stream);

		// Assert:
		ASSERT_EQ(Data_Size, buffer.size());

		const auto* pData32 = reinterpret_cast<const uint32_t*>(buffer.data());
		const auto* pData64 = reinterpret_cast<const uint64_t*>(buffer.data());
		EXPECT_EQ(chainHeight, Height(pData64[0]));
		EXPECT_EQ(data.ChainScore, model::ChainScore(pData64[1], pData64[2]));
		EXPECT_EQ(data.State.LastRecalculationHeight, model::ImportanceHeight(pData64[3]));
		EXPECT_EQ(data.State.LastFinalizedHeight, Height(pData64[4]));
		EXPECT_EQ(data.State.NumTotalTransactions, pData64[5]);
		EXPECT_EQ(data.State.DynamicFeeMultiplier, BlockFeeMultiplier(pData32[12]));

		EXPECT_EQ(1u, stream.numFlushes());
	}

	TEST(TEST_CLASS, CanLoadData) {
		// Arrange:
		SupplementalData data;
		Height chainHeight;

		std::vector<uint8_t> buffer(Data_Size);
		test::FillWithRandomData(buffer);
		mocks::MockMemoryStream stream(buffer);

		// Act:
		LoadSupplementalData(stream, data, chainHeight);

		// Assert:
		const auto* pData32 = reinterpret_cast<const uint32_t*>(buffer.data());
		const auto* pData64 = reinterpret_cast<const uint64_t*>(buffer.data());
		EXPECT_EQ(Height(pData64[0]), chainHeight);
		EXPECT_EQ(model::ChainScore(pData64[1], pData64[2]), data.ChainScore);
		EXPECT_EQ(model::ImportanceHeight(pData64[3]), data.State.LastRecalculationHeight);
		EXPECT_EQ(Height(pData64[4]), data.State.LastFinalizedHeight);
		EXPECT_EQ(pData64[5], data.State.NumTotalTransactions);
		EXPECT_EQ(BlockFeeMultiplier(pData32[12]), data.State.DynamicFeeMultiplier);
	}
}}

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

#include "LocalNodeApiTraits.h"
#include "catapult/model/EntityHasher.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	namespace {
		constexpr Height Invalid_Height(123456);
	}

	// region ChainStatisticsApiTraits

	ChainStatisticsApiTraits::RequestType ChainStatisticsApiTraits::InitiateValidRequest(const api::RemoteChainApi& remoteChainApi) {
		return remoteChainApi.chainStatistics();
	}

	void ChainStatisticsApiTraits::VerifyResult(const ResultType& chainStatistics) {
		EXPECT_EQ(Height(1), chainStatistics.Height);
		EXPECT_EQ(model::ChainScore(1), chainStatistics.Score);
	}

	// endregion

	// region HashesFromApiTraits

	HashesFromApiTraits::RequestType HashesFromApiTraits::InitiateValidRequest(const api::RemoteChainApi& remoteChainApi) {
		return remoteChainApi.hashesFrom(Height(1), 10);
	}

	HashesFromApiTraits::RequestType HashesFromApiTraits::InitiateInvalidRequest(const api::RemoteChainApi& remoteChainApi) {
		return remoteChainApi.hashesFrom(Height(0), 1);
	}

	void HashesFromApiTraits::VerifyResult(const ResultType& hashes) {
		mocks::MockMemoryBlockStorage storage;
		auto pBlock = storage.loadBlock(Height(1));
		auto expectedHash = model::CalculateHash(*pBlock);

		// checks ONLY first returned hash
		ASSERT_EQ(1u, hashes.size());
		EXPECT_EQ(expectedHash, *hashes.cbegin());
	}

	// endregion

	// region BlockAtApiTraits

	BlockAtApiTraits::RequestType BlockAtApiTraits::InitiateValidRequest(const api::RemoteChainApi& remoteChainApi) {
		return remoteChainApi.blockAt(Height(1));
	}

	BlockAtApiTraits::RequestType BlockAtApiTraits::InitiateInvalidRequest(const api::RemoteChainApi& remoteChainApi) {
		return remoteChainApi.blockAt(Invalid_Height);
	}

	void BlockAtApiTraits::VerifyResult(const ResultType& pBlock) {
		ASSERT_TRUE(!!pBlock);

		mocks::MockMemoryBlockStorage storage;
		auto pExpectedBlock = storage.loadBlock(Height(1));
		ASSERT_EQ(pExpectedBlock->Size, pBlock->Size);
		EXPECT_EQ(*pExpectedBlock, *pBlock);
	}

	// endregion

	// region BlockLastApiTraits

	BlockLastApiTraits::RequestType BlockLastApiTraits::InitiateValidRequest(const api::RemoteChainApi& remoteChainApi) {
		return remoteChainApi.blockLast();
	}

	void BlockLastApiTraits::VerifyResult(const ResultType& pBlock) {
		// right now this test is very artificial, it might be improved when we'll be able to push block
		ASSERT_TRUE(!!pBlock);

		mocks::MockMemoryBlockStorage storage;
		auto pExpectedBlock = storage.loadBlock(Height(1));
		ASSERT_EQ(pExpectedBlock->Size, pBlock->Size);
		EXPECT_EQ(*pExpectedBlock, *pBlock);
	}

	// endregion

	// region BlocksFromApiTraits

	BlocksFromApiTraits::RequestType BlocksFromApiTraits::InitiateValidRequest(const api::RemoteChainApi& remoteChainApi) {
		api::BlocksFromOptions options;
		options.NumBlocks = 10;
		options.NumBytes = 10 * 1024;
		return remoteChainApi.blocksFrom(Height(1), options);
	}

	void BlocksFromApiTraits::VerifyResult(const ResultType& blocks) {
		// right now this test is very artificial, it might be improved when we'll be able to push block
		EXPECT_EQ(1u, blocks.size());
	}

	// endregion
}}

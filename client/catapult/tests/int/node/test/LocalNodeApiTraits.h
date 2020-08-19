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

#pragma once
#include "catapult/api/RemoteChainApi.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Chain statistics api traits.
	struct ChainStatisticsApiTraits {
		using ResultType = api::ChainStatistics;
		using RequestType = decltype(std::declval<api::RemoteChainApi>().chainStatistics());

		/// Initiates a valid request using \a remoteChainApi.
		static RequestType InitiateValidRequest(const api::RemoteChainApi& remoteChainApi);

		/// Verifies the result \a chainStatistics.
		static void VerifyResult(const ResultType& chainStatistics);
	};

	/// Hashes from api traits.
	struct HashesFromApiTraits {
		using ResultType = model::HashRange;
		using RequestType = decltype(std::declval<api::RemoteChainApi>().hashesFrom(Height(), uint32_t()));

		/// Initiates a valid request using \a remoteChainApi.
		static RequestType InitiateValidRequest(const api::RemoteChainApi& remoteChainApi);

		/// Initiates an invalid request using \a remoteChainApi.
		static RequestType InitiateInvalidRequest(const api::RemoteChainApi& remoteChainApi);

		/// Verifies the result \a hashes.
		static void VerifyResult(const ResultType& hashes);
	};

	/// Block at api traits.
	struct BlockAtApiTraits {
		using ResultType = std::shared_ptr<const model::Block>;
		using RequestType = decltype(std::declval<api::RemoteChainApi>().blockAt(Height()));

		/// Initiates a valid request using \a remoteChainApi.
		static RequestType InitiateValidRequest(const api::RemoteChainApi& remoteChainApi);

		/// Initiates an invalid request using \a remoteChainApi.
		static RequestType InitiateInvalidRequest(const api::RemoteChainApi& remoteChainApi);

		/// Verifies the result \a pBlock.
		static void VerifyResult(const ResultType& pBlock);
	};

	/// Block last api traits.
	struct BlockLastApiTraits {
		using ResultType = std::shared_ptr<const model::Block>;
		using RequestType = decltype(std::declval<api::RemoteChainApi>().blockLast());

		/// Initiates a valid request using \a remoteChainApi.
		static RequestType InitiateValidRequest(const api::RemoteChainApi& remoteChainApi);

		/// Verifies the result \a pBlock.
		static void VerifyResult(const ResultType& pBlock);
	};

	/// Blocks from api traits.
	struct BlocksFromApiTraits {
		using ResultType = model::BlockRange;
		using RequestType = decltype(std::declval<api::RemoteChainApi>().blocksFrom(Height(), api::BlocksFromOptions()));

		/// Initiates a valid request using \a remoteChainApi.
		static RequestType InitiateValidRequest(const api::RemoteChainApi& remoteChainApi);

		/// Verifies the result \a blocks.
		static void VerifyResult(const ResultType& blocks);
	};
}}

/// Adds an integrity test \a TEST_NAME to \a TEST_CLASS for api \a API_NAME.
#define CHAIN_API_INT_ADD_API_TEST(TEST_CLASS, TEST_NAME, API_NAME) \
	TEST(TEST_CLASS, TEST_NAME##_##API_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::API_NAME##ApiTraits>(); } \

/// Adds \a TEST_NAME test for all chain apis.
#define CHAIN_API_INT_VALID_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TApiTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	CHAIN_API_INT_ADD_API_TEST(TEST_CLASS, TEST_NAME, ChainStatistics) \
	CHAIN_API_INT_ADD_API_TEST(TEST_CLASS, TEST_NAME, HashesFrom) \
	CHAIN_API_INT_ADD_API_TEST(TEST_CLASS, TEST_NAME, BlockAt) \
	CHAIN_API_INT_ADD_API_TEST(TEST_CLASS, TEST_NAME, BlockLast) \
	CHAIN_API_INT_ADD_API_TEST(TEST_CLASS, TEST_NAME, BlocksFrom) \
	template<typename TApiTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

/// Adds \a TEST_NAME test for all chain apis that can receive invalid input.
#define CHAIN_API_INT_INVALID_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TApiTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	CHAIN_API_INT_ADD_API_TEST(TEST_CLASS, TEST_NAME, HashesFrom) \
	CHAIN_API_INT_ADD_API_TEST(TEST_CLASS, TEST_NAME, BlockAt) \
	template<typename TApiTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

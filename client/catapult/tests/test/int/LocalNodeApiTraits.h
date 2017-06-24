#pragma once
#include "catapult/api/RemoteChainApi.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Chain info api traits.
	struct ChainInfoApiTraits {
		using ResultType = api::ChainInfo;
		using RequestType = decltype(std::declval<api::RemoteChainApi>().chainInfo());

		/// Initiates a valid request using \a remoteApi.
		static RequestType InitiateValidRequest(const api::RemoteChainApi& remoteApi);

		/// Verifies the result \a info.
		static void VerifyResult(const ResultType& info);
	};

	/// Hashes from api traits.
	struct HashesFromApiTraits {
		using ResultType = model::HashRange;
		using RequestType = decltype(std::declval<api::RemoteChainApi>().hashesFrom(Height()));

		/// Initiates a valid request using \a remoteApi.
		static RequestType InitiateValidRequest(const api::RemoteChainApi& remoteApi);

		/// Initiates an invalid request using \a remoteApi.
		static RequestType InitiateInvalidRequest(const api::RemoteChainApi& remoteApi);

		/// Verifies the result \a hashes.
		static void VerifyResult(const ResultType& hashes);
	};

	/// Block at api traits.
	struct BlockAtApiTraits {
		using ResultType = std::shared_ptr<const model::Block>;
		using RequestType = decltype(std::declval<api::RemoteChainApi>().blockAt(Height()));

		/// Initiates a valid request using \a remoteApi.
		static RequestType InitiateValidRequest(const api::RemoteChainApi& remoteApi);

		/// Initiates an invalid request using \a remoteApi.
		static RequestType InitiateInvalidRequest(const api::RemoteChainApi& remoteApi);

		/// Verifies the result \a pBlock.
		static void VerifyResult(const ResultType& pBlock);
	};

	/// Block last api traits.
	struct BlockLastApiTraits {
		using ResultType = std::shared_ptr<const model::Block>;
		using RequestType = decltype(std::declval<api::RemoteChainApi>().blockLast());

		/// Initiates a valid request using \a remoteApi.
		static RequestType InitiateValidRequest(const api::RemoteChainApi& remoteApi);

		/// Verifies the result \a pBlock.
		static void VerifyResult(const ResultType& pBlock);
	};

	/// Blocks from api traits.
	struct BlocksFromApiTraits {
		using ResultType = model::BlockRange;
		using RequestType = decltype(std::declval<api::RemoteChainApi>().blocksFrom(Height(), api::BlocksFromOptions()));

		/// Initiates a valid request using \a remoteApi.
		static RequestType InitiateValidRequest(const api::RemoteChainApi& remoteApi);

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
	CHAIN_API_INT_ADD_API_TEST(TEST_CLASS, TEST_NAME, ChainInfo) \
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

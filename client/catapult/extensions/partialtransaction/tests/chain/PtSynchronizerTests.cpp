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

#include "partialtransaction/src/chain/PtSynchronizer.h"
#include "partialtransaction/tests/test/mocks/MockPtApi.h"
#include "tests/test/other/EntitiesSynchronizerTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

	namespace {
		using MockRemoteApi = mocks::MockPtApi;

		class PtSynchronizerTraits {
		public:
			using RequestElementType = cache::ShortHashPair;
			using ResponseContainerType = partialtransaction::CosignedTransactionInfos;

		public:
			class RemoteApiWrapper {
			public:
				explicit RemoteApiWrapper(const ResponseContainerType& transactionInfos)
						: m_pTransactionApi(std::make_unique<MockRemoteApi>(transactionInfos))
				{}

			public:
				const auto& api() const {
					return *m_pTransactionApi;
				}

				auto numCalls() const {
					return m_pTransactionApi->transactionInfosRequests().size();
				}

				const auto& singleRequest() const {
					return m_pTransactionApi->transactionInfosRequests()[0].ShortHashPairs;
				}

				void setError(bool setError = true) {
					auto entryPoint = setError
							? MockRemoteApi::EntryPoint::Partial_Transaction_Infos
							: MockRemoteApi::EntryPoint::None;
					m_pTransactionApi->setError(entryPoint);
				}

				void checkAdditionalRequestParameters() {
					EXPECT_EQ(Timestamp(84), m_pTransactionApi->transactionInfosRequests()[0].Deadline);
				}

			private:
				std::unique_ptr<MockRemoteApi> m_pTransactionApi;
			};

		public:
			static auto CreateRequestRange(uint32_t count) {
				auto shortHashPairs = test::GenerateRandomDataVector<cache::ShortHashPair>(count);
				return cache::ShortHashPairRange::CopyFixed(reinterpret_cast<const uint8_t*>(shortHashPairs.data()), count);
			}

			static auto CreateResponseContainer(uint32_t count) {
				// Arrange: only populate hashes
				partialtransaction::CosignedTransactionInfos transactionInfos;
				for (auto i = 0u; i < count; ++i) {
					model::CosignedTransactionInfo transactionInfo;
					transactionInfo.EntityHash = test::GenerateRandomByteArray<Hash256>();
					transactionInfos.push_back(transactionInfo);
				}

				return transactionInfos;
			}

			static auto CreateRemoteApi(const ResponseContainerType& transactionInfos) {
				return RemoteApiWrapper(transactionInfos);
			}

			static auto CreateSynchronizer(
					const partialtransaction::ShortHashPairsSupplier& shortHashPairsSupplier,
					const partialtransaction::CosignedTransactionInfosConsumer& transactionInfosConsumer,
					bool shouldExecute = true) {
				return CreatePtSynchronizer(
						[]() { return Timestamp(84); },
						shortHashPairsSupplier,
						transactionInfosConsumer,
						[shouldExecute]() { return shouldExecute; });
			}

			static void AssertCustomResponse(const ResponseContainerType& expectedResponse, const ResponseContainerType& actualResponse) {
				// Assert: only compare hashes (because only hashes are set in CreateResponseContainer)
				ASSERT_EQ(expectedResponse.size(), actualResponse.size());
				for (auto i = 0u; i < expectedResponse.size(); ++i)
					EXPECT_EQ(expectedResponse[i].EntityHash, actualResponse[i].EntityHash) << "info at " << i;
			}
		};
	}

	DEFINE_CONDITIONAL_ENTITIES_SYNCHRONIZER_TESTS(PtSynchronizer)
}}

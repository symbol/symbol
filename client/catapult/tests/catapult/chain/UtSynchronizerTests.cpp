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

#include "catapult/chain/UtSynchronizer.h"
#include "tests/catapult/chain/test/MockTransactionApi.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/other/EntitiesSynchronizerTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

	namespace {
		using MockRemoteApi = mocks::MockTransactionApi;

		class UtSynchronizerTraits {
		public:
			using RequestElementType = utils::ShortHash;
			using ResponseContainerType = model::AnnotatedEntityRange<model::Transaction>;

		public:
			class RemoteApiWrapper {
			public:
				explicit RemoteApiWrapper(const model::TransactionRange& transactionRange)
						: m_pTransactionApi(std::make_unique<MockRemoteApi>(transactionRange))
				{}

			public:
				const auto& api() const {
					return *m_pTransactionApi;
				}

				auto numCalls() const {
					return m_pTransactionApi->utRequests().size();
				}

				const auto& singleRequest() const {
					return m_pTransactionApi->utRequests()[0].ShortHashes;
				}

				void setError(bool setError = true) {
					auto entryPoint = setError
							? MockRemoteApi::EntryPoint::Unconfirmed_Transactions
							: MockRemoteApi::EntryPoint::None;
					m_pTransactionApi->setError(entryPoint);
				}

				void checkAdditionalRequestParameters() {
					EXPECT_EQ(Timestamp(84), m_pTransactionApi->utRequests()[0].Deadline);
					EXPECT_EQ(BlockFeeMultiplier(17), m_pTransactionApi->utRequests()[0].FeeMultiplier);
				}

			private:
				std::unique_ptr<MockRemoteApi> m_pTransactionApi;
			};

		public:
			static auto CreateRequestRange(uint32_t count) {
				auto shortHashes = test::GenerateRandomDataVector<utils::ShortHash>(count);
				return model::ShortHashRange::CopyFixed(reinterpret_cast<const uint8_t*>(shortHashes.data()), count);
			}

			static auto CreateResponseContainer(uint32_t count) {
				return test::CreateTransactionEntityRange(count);
			}

			static auto CreateRemoteApi(const model::TransactionRange& transactionRange) {
				return RemoteApiWrapper(transactionRange);
			}

			static auto CreateSynchronizer(
					const ShortHashesSupplier& shortHashesSupplier,
					const handlers::TransactionRangeHandler& transactionRangeConsumer,
					bool shouldExecute = true) {
				return CreateUtSynchronizer(
						BlockFeeMultiplier(17),
						[]() { return Timestamp(84); },
						shortHashesSupplier,
						transactionRangeConsumer,
						[shouldExecute]() { return shouldExecute; });
			}
		};
	}

	DEFINE_CONDITIONAL_ENTITIES_SYNCHRONIZER_TESTS(UtSynchronizer)
}}

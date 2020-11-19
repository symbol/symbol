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

#include "catapult/chain/EntitiesSynchronizer.h"
#include "catapult/chain/RemoteNodeSynchronizer.h"
#include "catapult/handlers/HandlerTypes.h"
#include "tests/catapult/chain/test/MockChainApi.h"
#include "tests/test/core/HashTestUtils.h"
#include "tests/test/other/EntitiesSynchronizerTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

	namespace {
		using HashesSupplier = supplier<model::HashRange>;
		using HashRangeConsumerFunc = handlers::RangeHandler<Hash256>;

		// region api

		class MockHashApi : public api::RemoteApi {
		public:
			explicit MockHashApi(const model::HashRange& hashes)
					: RemoteApi({ test::GenerateRandomByteArray<Key>(), "fake-host-from-mock-hash-api" })
					, m_hashes(model::HashRange::CopyRange(hashes))
					, m_hasError(false)
			{}

		public:
			void setError(bool hasError) {
				m_hasError = hasError;
			}

			const std::vector<model::HashRange>& hashesRequests() const {
				return m_hashesRequests;
			}

		public:
			thread::future<model::HashRange> hashes(model::HashRange&& hashes) const {
				m_hashesRequests.push_back(std::move(hashes));
				if (m_hasError)
					return CreateFutureException<model::HashRange>("hashes error has been set");

				return thread::make_ready_future(model::HashRange::CopyRange(m_hashes));
			}

		private:
			template<typename T>
			static thread::future<T> CreateFutureException(const char* message) {
				return thread::make_exceptional_future<T>(catapult_runtime_error(message));
			}

		private:
			model::HashRange m_hashes;
			bool m_hasError;
			mutable std::vector<model::HashRange> m_hashesRequests;
		};

		// endregion

		// region fake synchronizer (that sends and receives hashes)

		struct HashesTraits {
		public:
			using RemoteApiType = MockHashApi;
			static constexpr auto Name = "hashes";

		public:
			HashesTraits(const HashesSupplier& hashesSupplier, const HashRangeConsumerFunc& hashRangeConsumer)
					: m_hashesSupplier(hashesSupplier)
					, m_hashRangeConsumer(hashRangeConsumer)
			{}

		public:
			thread::future<model::HashRange> apiCall(const RemoteApiType& api) const {
				return api.hashes(m_hashesSupplier());
			}

			void consume(model::HashRange&& range, const model::NodeIdentity& sourceIdentity) const {
				m_hashRangeConsumer(model::AnnotatedEntityRange<Hash256>(std::move(range), sourceIdentity));
			}

		private:
			HashesSupplier m_hashesSupplier;
			HashRangeConsumerFunc m_hashRangeConsumer;
		};

		RemoteNodeSynchronizer<MockHashApi> CreateHashesSynchronizer(
				const HashesSupplier& hashesSupplier,
				const HashRangeConsumerFunc& hashRangeConsumer) {
			auto traits = HashesTraits(hashesSupplier, hashRangeConsumer);
			auto pSynchronizer = std::make_shared<EntitiesSynchronizer<HashesTraits>>(std::move(traits));
			return CreateRemoteNodeSynchronizer(pSynchronizer);
		}

		// endregion

		class EntitiesSynchronizerTraits {
		public:
			using RequestElementType = Hash256;
			using ResponseContainerType = model::AnnotatedEntityRange<Hash256>;

		public:
			class RemoteApiWrapper {
			public:
				explicit RemoteApiWrapper(const model::HashRange& hashRange) : m_pHashApi(std::make_unique<MockHashApi>(hashRange))
				{}

			public:
				const auto& api() const {
					return *m_pHashApi;
				}

				auto numCalls() const {
					return m_pHashApi->hashesRequests().size();
				}

				const auto& singleRequest() const {
					return m_pHashApi->hashesRequests()[0];
				}

				void setError(bool setError = true) {
					m_pHashApi->setError(setError);
				}

				void checkAdditionalRequestParameters() {
					// no additional request parameters
				}

			private:
				std::unique_ptr<MockHashApi> m_pHashApi;
			};

		public:
			static auto CreateRequestRange(uint32_t count) {
				return test::GenerateRandomHashes(count);
			}

			static auto CreateResponseContainer(uint32_t count) {
				return test::GenerateRandomHashes(count);
			}

			static auto CreateRemoteApi(const model::HashRange& hashRange) {
				return RemoteApiWrapper(hashRange);
			}

			static auto CreateSynchronizer(const HashesSupplier& hashesSupplier, const HashRangeConsumerFunc& hashRangeConsumer) {
				return CreateHashesSynchronizer(hashesSupplier, hashRangeConsumer);
			}
		};
	}

	DEFINE_ENTITIES_SYNCHRONIZER_TESTS(EntitiesSynchronizer)
}}

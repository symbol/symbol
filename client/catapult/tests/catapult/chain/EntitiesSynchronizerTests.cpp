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

		class MockHashApi {
		public:
			explicit MockHashApi(const model::HashRange& hashes)
					: m_hashes(model::HashRange::CopyRange(hashes))
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
			explicit HashesTraits(const HashesSupplier& hashesSupplier, const HashRangeConsumerFunc& hashRangeConsumer)
					: m_hashesSupplier(hashesSupplier)
					, m_hashRangeConsumer(hashRangeConsumer)
			{}

		public:
			thread::future<model::HashRange> apiCall(const RemoteApiType& api) const {
				return api.hashes(m_hashesSupplier());
			}

			void consume(model::HashRange&& range) const {
				m_hashRangeConsumer(std::move(range));
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
				explicit RemoteApiWrapper(const model::HashRange& hashRange) : m_hashApi(hashRange)
				{}

			public:
				const auto& api() const {
					return m_hashApi;
				}

				auto numCalls() const {
					return m_hashApi.hashesRequests().size();
				}

				const auto& singleRequest() const {
					return m_hashApi.hashesRequests()[0];
				}

				void setError(bool setError = true) {
					m_hashApi.setError(setError);
				}

			private:
				MockHashApi m_hashApi;
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

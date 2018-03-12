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
					return m_pTransactionApi->utRequests()[0];
				}

				void setError(bool setError = true) {
					auto entryPoint = setError
							? MockRemoteApi::EntryPoint::Unconfirmed_Transactions
							: MockRemoteApi::EntryPoint::None;
					m_pTransactionApi->setError(entryPoint);
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
					const handlers::TransactionRangeHandler& transactionRangeConsumer) {
				return CreateUtSynchronizer(shortHashesSupplier, transactionRangeConsumer);
			}
		};
	}

	DEFINE_ENTITIES_SYNCHRONIZER_TESTS(UtSynchronizer)
}}

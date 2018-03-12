#include "src/observers/Observers.h"
#include "plugins/txes/aggregate/src/model/AggregateEntityType.h"
#include "tests/test/LockStatusAndBalanceObserverTests.h"
#include "tests/test/plugins/ObserverTestUtils.h"

namespace catapult { namespace observers {

#define TEST_CLASS CompletedAggregateObserverTests

	using ObserverTestContext = test::ObserverTestContextT<test::HashLockInfoCacheFactory>;

	DEFINE_COMMON_OBSERVER_TESTS(CompletedAggregate,)

	namespace {
		struct HashTraits {
		public:
			using BasicTraits = test::BasicHashLockInfoTestTraits;

			struct NotificationBuilder {
			public:
				NotificationBuilder()
						: m_entityType(model::Entity_Type_Aggregate_Bonded)
						, m_deadline(test::GenerateRandomValue<Timestamp>()) {
					test::FillWithRandomData(m_signer);
					test::FillWithRandomData(m_transactionHash);
				}

			public:
				auto notification() {
					return model::TransactionNotification(m_signer, m_transactionHash, m_entityType, m_deadline);
				}

				void setHash(const Hash256& hash) {
					m_transactionHash = hash;
				}

				void setType(model::EntityType type) {
					m_entityType = type;
				}

			private:
				model::EntityType m_entityType;
				Timestamp m_deadline;
				Key m_signer;
				Hash256 m_transactionHash;
			};

			using ObserverTestContext = observers::ObserverTestContext;

			static auto CreateObserver() {
				return CreateCompletedAggregateObserver();
			}

			static auto DestinationAccount(const BasicTraits::ValueType& lockInfo) {
				return lockInfo.Account;
			}
		};

		void AssertObserverIgnoresUnknownTransactionType(model::EntityType entityType) {
			// Arrange: cache is empty
			ObserverTestContext context(observers::NotifyMode::Commit);
			auto pObserver = HashTraits::CreateObserver();

			// Act:
			HashTraits::NotificationBuilder notificationBuilder;
			notificationBuilder.setType(entityType);

			test::ObserveNotification(*pObserver, notificationBuilder.notification(), context);

			// Assert: observer would have thrown if it had accessed the cache
			auto& lockInfoCacheDelta = context.cache().sub<HashTraits::BasicTraits::CacheType>();
			EXPECT_EQ(0u, lockInfoCacheDelta.size()) << entityType;
		}
	}

	TEST(TEST_CLASS, ObserverIgnoresUnknownTransactionType) {
		// Assert:
		for (auto type : { model::Entity_Type_Aggregate_Complete, static_cast<model::EntityType>(0xFFFF) })
			AssertObserverIgnoresUnknownTransactionType(type);
	}

	DEFINE_LOCK_STATUS_OBSERVER_TESTS(HashTraits)
}}

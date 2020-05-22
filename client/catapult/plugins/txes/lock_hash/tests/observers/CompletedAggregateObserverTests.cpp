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

#include "src/observers/Observers.h"
#include "src/model/HashLockReceiptType.h"
#include "plugins/txes/aggregate/src/model/AggregateEntityType.h"
#include "plugins/txes/lock_shared/tests/observers/LockStatusAndBalanceObserverTests.h"
#include "tests/test/HashLockInfoCacheTestUtils.h"
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
					test::FillWithRandomData(m_sender);
					test::FillWithRandomData(m_transactionHash);
				}

			public:
				auto notification() {
					return model::TransactionNotification(m_sender, m_transactionHash, m_entityType, m_deadline);
				}

				void setType(model::EntityType type) {
					m_entityType = type;
				}

				void prepare(const state::HashLockInfo& lockInfo) {
					m_transactionHash = lockInfo.Hash;
				}

			private:
				model::EntityType m_entityType;
				Timestamp m_deadline;
				Address m_sender;
				Hash256 m_transactionHash;
			};

			using ObserverTestContext = observers::ObserverTestContext;

			static constexpr auto Receipt_Type = model::Receipt_Type_LockHash_Completed;

			static auto CreateObserver() {
				return CreateCompletedAggregateObserver();
			}

			static auto DestinationAccount(const BasicTraits::ValueType& lockInfo) {
				return lockInfo.OwnerAddress;
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
		for (auto type : { model::Entity_Type_Aggregate_Complete, static_cast<model::EntityType>(0xFFFF) })
			AssertObserverIgnoresUnknownTransactionType(type);
	}

	DEFINE_LOCK_STATUS_OBSERVER_TESTS(HashTraits)
}}

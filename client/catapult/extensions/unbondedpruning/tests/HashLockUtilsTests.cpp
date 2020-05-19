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

#include "unbondedpruning/src/HashLockUtils.h"
#include "plugins/txes/lock_hash/src/model/HashLockNotifications.h"
#include "catapult/model/NotificationPublisher.h"
#include "catapult/model/NotificationSubscriber.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace unbondedpruning {

#define TEST_CLASS HashLockUtilsTests

	namespace {
		class MockNotificationPublisher : public model::NotificationPublisher {
		public:
			explicit MockNotificationPublisher(const utils::HashSet& dependentHashes) : m_dependentHashes(dependentHashes)
			{}

		public:
			const auto& entityInfos() const {
				return m_entityInfos;
			}

		public:
			void publish(const model::WeakEntityInfo& entityInfo, model::NotificationSubscriber& sub) const override {
				m_entityInfos.push_back(entityInfo);

				// raise one hash lock notification for each dependent hash
				for (const auto& hash : m_dependentHashes)
					sub.notify(model::HashLockNotification(Address(), model::UnresolvedMosaic(), BlockDuration(), hash));

				// if there are no dependent hashes, don't raise any notifications
			}

		private:
			utils::HashSet m_dependentHashes;
			mutable std::vector<model::WeakEntityInfo> m_entityInfos;
		};

		void AssertDependentTransactionHashesExtraction(
				size_t numExpectedDependentHashes,
				const utils::HashSet& publishedDependentHashes) {
			// Arrange:
			auto transactionInfo = test::CreateRandomTransactionInfo();
			MockNotificationPublisher publisher(publishedDependentHashes);

			// Act:
			auto dependentHashes = FindDependentTransactionHashes(transactionInfo, publisher);

			// Assert: publish was called exactly once
			ASSERT_EQ(1u, publisher.entityInfos().size());
			EXPECT_EQ(model::WeakEntityInfo(*transactionInfo.pEntity, transactionInfo.EntityHash), publisher.entityInfos()[0]);

			// - the expected dependent hashes were extracted
			EXPECT_EQ(numExpectedDependentHashes, dependentHashes.size());
			EXPECT_EQ(publishedDependentHashes, dependentHashes);
		}
	}

	TEST(TEST_CLASS, DependentHashIsExtractedFromLockHashNotifications) {
		AssertDependentTransactionHashesExtraction(1, { test::GenerateRandomByteArray<Hash256>() });
	}

	TEST(TEST_CLASS, MultipleDependentHashesCanBeExtractedFromSingleTransaction) {
		AssertDependentTransactionHashesExtraction(3, {
			test::GenerateRandomByteArray<Hash256>(),
			test::GenerateRandomByteArray<Hash256>(),
			test::GenerateRandomByteArray<Hash256>()
		});
	}

	TEST(TEST_CLASS, NoDependentHashesAreExtractedFromOtherNotifications) {
		AssertDependentTransactionHashesExtraction(0, utils::HashSet());
	}
}}

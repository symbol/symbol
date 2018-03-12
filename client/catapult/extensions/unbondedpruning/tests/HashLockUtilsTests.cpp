#include "unbondedpruning/src/HashLockUtils.h"
#include "plugins/txes/lock/src/model/LockNotifications.h"
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
					sub.notify(model::HashLockNotification(Key(), model::Mosaic(), BlockDuration(), hash));

				// if there are no dependent hashes, raise a different notification that should be ignored
				if (m_dependentHashes.empty())
					sub.notify(model::SecretLockHashAlgorithmNotification(model::LockHashAlgorithm::Op_Sha3));
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
		// Assert:
		AssertDependentTransactionHashesExtraction(1, { test::GenerateRandomData<Hash256_Size>() });
	}

	TEST(TEST_CLASS, MultipleDependentHashesCanBeExtractedFromSingleTransaction) {
		// Assert:
		AssertDependentTransactionHashesExtraction(3, {
			test::GenerateRandomData<Hash256_Size>(),
			test::GenerateRandomData<Hash256_Size>(),
			test::GenerateRandomData<Hash256_Size>()
		});
	}

	TEST(TEST_CLASS, NoDependentHashesAreExtractedFromOtherNotifications) {
		// Assert:
		AssertDependentTransactionHashesExtraction(0, utils::HashSet());
	}
}}

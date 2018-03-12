#include "unbondedpruning/src/UnbondedPruningService.h"
#include "plugins/txes/lock/src/model/LockNotifications.h"
#include "catapult/consumers/BlockChainSyncHandlers.h"
#include "catapult/model/NotificationSubscriber.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/test/core/mocks/MockTransactionPluginUnsupported.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace unbondedpruning {

#define TEST_CLASS UnbondedPruningServiceTests

	namespace {
		struct UnbondedPruningServiceTraits {
			static constexpr auto CreateRegistrar = CreateUnbondedPruningServiceRegistrar;
		};

		using TestContext = test::ServiceLocatorTestContext<UnbondedPruningServiceTraits>;
	}

	ADD_SERVICE_REGISTRAR_INFO_TEST(UnbondedPruning, Post_Transaction_Event_Handlers)

	TEST(TEST_CLASS, NoServicesOrCountersAreRegistered) {
		// Assert:
		test::AssertNoServicesOrCountersAreRegistered<TestContext>();
	}

	// region addTransactionsChangeHandler

	namespace {
		Hash256 MutateHash(uint8_t firstByte, const Hash256& hash) {
			auto hashCopy = hash;
			hashCopy[0] = firstByte;
			return hashCopy;
		}

		class MockTransactionPlugin : public mocks::MockTransactionPluginUnsupported {
		public:
			explicit MockTransactionPlugin(const utils::HashSet& dependentHashes)
					: m_dependentHashes(dependentHashes)
					, m_numPublishes(0)
			{}

		public:
			void publish(const model::WeakEntityInfoT<model::Transaction>&, model::NotificationSubscriber& sub) const override {
				for (const auto& hash : m_dependentHashes)
					sub.notify(model::HashLockNotification(Key(), model::Mosaic(), BlockDuration(), MutateHash(m_numPublishes, hash)));

				++m_numPublishes;
			}

		private:
			utils::HashSet m_dependentHashes;
			mutable uint8_t m_numPublishes;
		};

		void AssertDependencyRemovedTransactionEvents(
				size_t numTransactions,
				const utils::HashSet& publishedDependentHashes,
				const utils::HashSet& expectedDependentHashes) {
			// Arrange:
			TestContext context;
			auto& hooks = context.testState().state().hooks();

			// - register a custom mock transaction plugin
			context.testState().pluginManager().addTransactionSupport(std::make_unique<MockTransactionPlugin>(publishedDependentHashes));

			// - collect all dependent hashes
			utils::HashSet collectedDependentHashes;
			std::vector<extensions::TransactionEvent> collectedEvents;
			hooks.addTransactionEventHandler([&collectedDependentHashes, &collectedEvents](const auto& eventData) {
				collectedDependentHashes.insert(eventData.TransactionHash);
				collectedEvents.push_back(eventData.Event);
			});

			// - boot the service
			context.boot();
			auto handler = hooks.transactionsChangeHandler();

			// Act: trigger a transactions change event (prepare some ignored addedTransactionHashes)
			auto addedTransactionHashes = test::GenerateRandomDataVector<Hash256>(2);
			utils::HashPointerSet addedTransactionHashPointers{ &addedTransactionHashes[0], &addedTransactionHashes[1] };
			auto revertedTransactionInfos = test::CreateTransactionInfos(numTransactions);
			auto changeInfo = consumers::TransactionsChangeInfo(addedTransactionHashPointers, revertedTransactionInfos);
			handler(changeInfo);

			// Assert:
			// - the expected number of events were collected
			EXPECT_EQ(numTransactions * publishedDependentHashes.size(), collectedEvents.size());
			EXPECT_EQ(expectedDependentHashes.size(), collectedEvents.size());
			EXPECT_EQ(expectedDependentHashes.size(), collectedDependentHashes.size());

			// - all collected events are Dependency_Removed
			auto i = 0u;
			for (auto event : collectedEvents) {
				EXPECT_EQ(extensions::TransactionEvent::Dependency_Removed, event) << "event at " << i;
				++i;
			}

			// - the collected hashes are correct
			EXPECT_EQ(expectedDependentHashes, collectedDependentHashes);
		}
	}

	TEST(TEST_CLASS, NoTransactionEventsAreRaisedWhenNoDependentHashesAreFound_SingleTransaction) {
		// Assert: 1 transaction x 0 dependent hash => 0 collected dependent hash
		AssertDependencyRemovedTransactionEvents(1, utils::HashSet(), utils::HashSet());
	}

	TEST(TEST_CLASS, TransactionEventsAreRaisedWhenDependentHashesAreFound_SingleTransactionSingleDependency) {
		// Assert: 1 transaction x 1 dependent hash => 1 collected dependent hash
		auto hash = test::GenerateRandomData<Hash256_Size>();
		AssertDependencyRemovedTransactionEvents(1, { hash }, { MutateHash(0, hash) });
	}

	TEST(TEST_CLASS, TransactionEventsAreRaisedWhenDependentHashesAreFound_SingleTransactionMultipleDependencies) {
		// Assert: 1 transaction x 3 dependent hash => 3 collected dependent hash
		auto hashes = test::GenerateRandomDataVector<Hash256>(3);
		AssertDependencyRemovedTransactionEvents(1, { hashes[0], hashes[1], hashes[2] }, {
			MutateHash(0, hashes[0]), MutateHash(0, hashes[1]), MutateHash(0, hashes[2])
		});
	}

	TEST(TEST_CLASS, NoTransactionEventsAreRaisedWhenNoDependentHashesAreFound_MultipleTransactions) {
		// Assert: 3 transaction x 0 dependent hash => 0 collected dependent hash
		AssertDependencyRemovedTransactionEvents(3, utils::HashSet(), utils::HashSet());
	}

	TEST(TEST_CLASS, TransactionEventsAreRaisedWhenDependentHashesAreFound_MultipleTransactionsSingleDependency) {
		// Assert: 3 transaction x 1 dependent hash => 3 collected dependent hash
		auto hash = test::GenerateRandomData<Hash256_Size>();
		AssertDependencyRemovedTransactionEvents(3, { hash }, {
			MutateHash(0, hash), MutateHash(1, hash), MutateHash(2, hash)
		});
	}

	TEST(TEST_CLASS, TransactionEventsAreRaisedWhenDependentHashesAreFound_MultipleTransactionsMultipleDependencies) {
		// Assert: 2 transaction x 3 dependent hash => 6 collected dependent hash
		auto hashes = test::GenerateRandomDataVector<Hash256>(3);
		AssertDependencyRemovedTransactionEvents(2, { hashes[0], hashes[1], hashes[2] }, {
			MutateHash(0, hashes[0]), MutateHash(0, hashes[1]), MutateHash(0, hashes[2]),
			MutateHash(1, hashes[0]), MutateHash(1, hashes[1]), MutateHash(1, hashes[2])
		});
	}

	// endregion
}}

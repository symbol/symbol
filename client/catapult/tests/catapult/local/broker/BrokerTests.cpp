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

#include "catapult/local/broker/Broker.h"
#include "catapult/extensions/ProcessBootstrapper.h"
#include "catapult/subscribers/SubscriberOperationTypes.h"
#include "tests/catapult/local/broker/test/BrokerTestUtils.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/local/MessageIngestionTestContext.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS BrokerTests

	// region test context

	class BrokerTestContext : public test::MessageIngestionTestContext {
	public:
		Broker& broker() const {
			return *m_pBroker;
		}

	public:
		void boot() {
			auto config = test::CreatePrototypicalCatapultConfiguration(dataDirectory().rootDir().str());
			auto pBootstrapper = std::make_unique<extensions::ProcessBootstrapper>(
					std::move(config),
					resourcesDirectory(),
					extensions::ProcessDisposition::Production,
					"BrokerTests");

			m_pBroker = CreateBroker(std::move(pBootstrapper));
		}

		void reset() {
			m_pBroker.reset();
		}

	private:
		std::unique_ptr<Broker> m_pBroker;
	};

	// endregion

	// region basic tests

	TEST(TEST_CLASS, CanBootBroker) {
		// Arrange:
		BrokerTestContext context;

		// Act + Assert: no exeception
		context.boot();
	}

	TEST(TEST_CLASS, CanShutdownBroker) {
		// Arrange:
		BrokerTestContext context;
		context.boot();

		// Act + Assert: no exception
		context.broker().shutdown();
	}

	// endregion

	// region ingestion - traits

	namespace {
		struct BlockChangeTraits {
			static constexpr auto Queue_Directory_Name = "block_change";

			static void WriteMessage(io::OutputStream& outputStream) {
				io::Write8(outputStream, utils::to_underlying_type(subscribers::BlockChangeOperationType::Drop_Blocks_After));
				io::Write64(outputStream, test::Random());
			}
		};

		struct UtChangeTraits {
			static constexpr auto Queue_Directory_Name = "unconfirmed_transactions_change";

			static constexpr auto WriteMessage = test::WriteRandomUtChange;
		};

		struct PtChangeTraits {
			static constexpr auto Queue_Directory_Name = "partial_transactions_change";

			static constexpr auto WriteMessage = test::WriteRandomPtChange;
		};

		struct FinalizationTraits {
			static constexpr auto Queue_Directory_Name = "finalization";

			static constexpr auto WriteMessage = test::WriteRandomFinalization;
		};

		struct StateChangeTraits {
			static constexpr auto Queue_Directory_Name = "state_change";

			static void WriteMessage(io::OutputStream& outputStream) {
				io::Write8(outputStream, utils::to_underlying_type(subscribers::StateChangeOperationType::Score_Change));
				io::Write64(outputStream, test::Random());
				io::Write64(outputStream, test::Random());
			}
		};

		struct TransactionStatusTraits {
			static constexpr auto Queue_Directory_Name = "transaction_status";

			static constexpr auto WriteMessage = test::WriteRandomTransactionStatus;
		};
	}

#define SUBSCRIBER_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_BlockChange) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BlockChangeTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_UtChange) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<UtChangeTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_PtChange) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PtChangeTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Finalization) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<FinalizationTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_StateChange) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<StateChangeTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_TransactionStatus) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransactionStatusTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region ingestion - tests

	SUBSCRIBER_TRAITS_BASED_TEST(CanIngestMessagesPresentWhenBooted) {
		// Arrange:
		BrokerTestContext context;

		// Act + Assert:
		test::ProduceAndConsumeMessages<TTraits>(context, 7);
	}

	SUBSCRIBER_TRAITS_BASED_TEST(CanIngestMessagesPresentWhenBootedStartingAtArbitraryIndex) {
		// Arrange: produce and consume some messages
		BrokerTestContext context;
		test::ProduceAndConsumeMessages<TTraits>(context, 7);

		// - reset broker in order to prevent further message digestion
		context.reset();

		// Act + Assert: produce and consume more messages (so ingestion starts at not the first message)
		// (second call to context.boot() will create a new broker)
		test::ProduceAndConsumeMessages<TTraits>(context, 5, 7);
	}

	SUBSCRIBER_TRAITS_BASED_TEST(CanIngestMessagesProducedWhileRunning) {
		// Arrange: produce and consume some messages
		BrokerTestContext context;
		test::ProduceAndConsumeMessages<TTraits>(context, 3);

		// Act: write more messages (broker is running)
		test::WriteMessages<TTraits>(context, 5);

		// Assert:
		WAIT_FOR_ZERO_EXPR(context.countMessageFiles(TTraits::Queue_Directory_Name));
		EXPECT_EQ(8u, context.readIndexReaderFile(TTraits::Queue_Directory_Name));
	}

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wused-but-marked-unused"
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif

	SUBSCRIBER_TRAITS_BASED_TEST(CannotIngestCorruptMessage) {
		// Arrange:
		BrokerTestContext context;
		ASSERT_DEATH({
			// - write some messages
			test::WriteMessages<TTraits>(context, 3);

			// - write poison message
			context.writeMessages(TTraits::Queue_Directory_Name, 1, [](auto& outputStream) {
				io::Write8(outputStream, 123);
			});

			// Sanity:
			EXPECT_EQ(4u, context.countMessageFiles(TTraits::Queue_Directory_Name));

			// Act: wait for them to be processed
			context.boot();
			WAIT_FOR_ZERO_EXPR(context.countMessageFiles(TTraits::Queue_Directory_Name));
		}, "");

		// Assert: only poison message remains
		WAIT_FOR_ONE_EXPR(context.countMessageFiles(TTraits::Queue_Directory_Name));
		EXPECT_EQ(3u, context.readIndexReaderFile(TTraits::Queue_Directory_Name));
	}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

	// endregion
}}

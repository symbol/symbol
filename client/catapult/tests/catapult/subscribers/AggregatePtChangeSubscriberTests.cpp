#include "catapult/subscribers/AggregatePtChangeSubscriber.h"
#include "tests/catapult/subscribers/test/AggregateSubscriberTestContext.h"
#include "tests/catapult/subscribers/test/UnsupportedSubscribers.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace subscribers {

#define TEST_CLASS AggregatePtChangeSubscriberTests

	namespace {
		using UnsupportedPtChangeSubscriber = test::UnsupportedPtChangeSubscriber<test::UnsupportedFlushBehavior::Throw>;

		template<typename TPtChangeSubscriber>
		using TestContext = test::AggregateSubscriberTestContext<TPtChangeSubscriber, AggregatePtChangeSubscriber<TPtChangeSubscriber>>;
	}

	TEST(TEST_CLASS, NotifyAddPartialsForwardsToAllSubscribers) {
		// Arrange:
		DEFINE_MOCK_INFOS_CAPTURE(PtChangeSubscriber, notifyAddPartials);

		TestContext<MockPtChangeSubscriber> context;
		auto transactionInfos = test::CopyTransactionInfosToSet(test::CreateTransactionInfos(3));

		// Sanity:
		EXPECT_EQ(3u, context.subscribers().size());

		// Act:
		context.aggregate().notifyAddPartials(transactionInfos);

		// Assert:
		test::AssertInfosDelegation(context, transactionInfos);
	}

	TEST(TEST_CLASS, NotifyAddCosignatureForwardsToAllSubscribers) {
		// Arrange:
		class MockPtChangeSubscriber : public UnsupportedPtChangeSubscriber {
		public:
			struct Param {
				Param(const model::TransactionInfo& parentTransactionInfo, const Key& signer, const Signature& signature)
						: ParentTransactionInfo(parentTransactionInfo)
						, Signer(signer)
						, Signature(signature)
				{}

				const model::TransactionInfo& ParentTransactionInfo;
				const Key& Signer;
				const catapult::Signature& Signature;
			};

		public:
			void notifyAddCosignature(const model::TransactionInfo& parentInfo, const Key& signer, const Signature& signature) override {
				CapturedParams.emplace_back(parentInfo, signer, signature);
			}

		public:
			std::vector<Param> CapturedParams;
		};

		TestContext<MockPtChangeSubscriber> context;
		auto transactionInfo = test::CreateRandomTransactionInfo();
		auto signer = test::GenerateRandomData<Key_Size>();
		auto signature = test::GenerateRandomData<Signature_Size>();

		// Sanity:
		EXPECT_EQ(3u, context.subscribers().size());

		// Act:
		context.aggregate().notifyAddCosignature(transactionInfo, signer, signature);

		// Assert:
		auto i = 0u;
		for (const auto* pSubscriber : context.subscribers()) {
			auto message = "subscriber at " + std::to_string(i++);
			const auto& capturedParams = pSubscriber->CapturedParams;
			ASSERT_EQ(1u, capturedParams.size()) << message;
			EXPECT_EQ(&transactionInfo, &capturedParams[0].ParentTransactionInfo) << message;
			EXPECT_EQ(&signer, &capturedParams[0].Signer) << message;
			EXPECT_EQ(&signature, &capturedParams[0].Signature) << message;
		}
	}

	TEST(TEST_CLASS, NotifyRemovePartialsForwardsToAllSubscribers) {
		// Arrange:
		DEFINE_MOCK_INFOS_CAPTURE(PtChangeSubscriber, notifyRemovePartials);

		TestContext<MockPtChangeSubscriber> context;
		auto transactionInfos = test::CopyTransactionInfosToSet(test::CreateTransactionInfos(3));

		// Sanity:
		EXPECT_EQ(3u, context.subscribers().size());

		// Act:
		context.aggregate().notifyRemovePartials(transactionInfos);

		// Assert:
		test::AssertInfosDelegation(context, transactionInfos);
	}

	TEST(TEST_CLASS, FlushForwardsToAllSubscribers) {
		// Arrange:
		DEFINE_MOCK_FLUSH_CAPTURE(PtChangeSubscriber);

		TestContext<MockPtChangeSubscriber> context;

		// Sanity:
		EXPECT_EQ(3u, context.subscribers().size());

		// Act:
		context.aggregate().flush();

		// Assert:
		test::AssertFlushDelegation(context);
	}
}}

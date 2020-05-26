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

#include "catapult/subscribers/AggregatePtChangeSubscriber.h"
#include "catapult/model/Cosignature.h"
#include "tests/catapult/subscribers/test/AggregateSubscriberTestContext.h"
#include "tests/catapult/subscribers/test/UnsupportedSubscribers.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
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
				Param(const model::TransactionInfo& parentTransactionInfo, const model::Cosignature& cosignature)
						: ParentTransactionInfo(parentTransactionInfo)
						, Cosignature(cosignature)
				{}

				const model::TransactionInfo& ParentTransactionInfo;
				const model::Cosignature& Cosignature;
			};

		public:
			void notifyAddCosignature(const model::TransactionInfo& parentInfo, const model::Cosignature& cosignature) override {
				CapturedParams.emplace_back(parentInfo, cosignature);
			}

		public:
			std::vector<Param> CapturedParams;
		};

		TestContext<MockPtChangeSubscriber> context;
		auto transactionInfo = test::CreateRandomTransactionInfo();
		auto cosignature = test::CreateRandomDetachedCosignature();

		// Sanity:
		EXPECT_EQ(3u, context.subscribers().size());

		// Act:
		context.aggregate().notifyAddCosignature(transactionInfo, cosignature);

		// Assert:
		auto i = 0u;
		for (const auto* pSubscriber : context.subscribers()) {
			auto message = "subscriber at " + std::to_string(i++);
			const auto& capturedParams = pSubscriber->CapturedParams;
			ASSERT_EQ(1u, capturedParams.size()) << message;
			EXPECT_EQ(&transactionInfo, &capturedParams[0].ParentTransactionInfo) << message;
			EXPECT_EQ(&cosignature, &capturedParams[0].Cosignature) << message;
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

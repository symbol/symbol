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

#include "zeromq/src/ZeroMqFinalizationSubscriber.h"
#include "zeromq/src/ZeroMqEntityPublisher.h"
#include "zeromq/tests/test/ZeroMqTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace zeromq {

#define TEST_CLASS ZeroMqFinalizationSubscriberTests

	namespace {
		class MqSubscriberContext : public test::MqContextT<subscribers::FinalizationSubscriber> {
		public:
			MqSubscriberContext() : MqContextT(CreateZeroMqFinalizationSubscriber)
			{}

		public:
			void notifyFinalizedBlock(Height height, const Hash256& hash, FinalizationPoint point) {
				subscriber().notifyFinalizedBlock(height, hash, point);
			}
		};
	}

	TEST(TEST_CLASS, SubscriberDoesNotReceiveDataOnDifferentTopic) {
		// Arrange:
		uint64_t topic(0x12345678);
		MqSubscriberContext context;
		context.subscribe(topic);

		auto hash = test::GenerateRandomByteArray<Hash256>();

		// Act:
		context.notifyFinalizedBlock(Height(123), hash, FinalizationPoint(55));

		// Assert:
		test::AssertNoPendingMessages(context.zmqSocket());
	}

	TEST(TEST_CLASS, CanNotifyFinalizedBlock) {
		// Arrange:
		MqSubscriberContext context;
		context.subscribe(BlockMarker::Finalized_Block_Marker);

		auto hash = test::GenerateRandomByteArray<Hash256>();

		// Act:
		context.notifyFinalizedBlock(Height(123), hash, FinalizationPoint(55));

		// Assert:
		zmq::multipart_t message;
		test::ZmqReceive(message, context.zmqSocket());

		test::AssertFinalizedBlockMessage(message, Height(123), hash, FinalizationPoint(55));
		test::AssertNoPendingMessages(context.zmqSocket());
	}
}}

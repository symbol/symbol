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

#include "catapult/consumers/ConsumerResultFactory.h"
#include "tests/TestHarness.h"

namespace catapult { namespace consumers {

#define TEST_CLASS ConsumerResultFactoryTests

	TEST(TEST_CLASS, CanCreateContinueConsumerResult) {
		// Act:
		auto result = Continue();

		// Assert:
		EXPECT_EQ(disruptor::CompletionStatus::Normal, result.CompletionStatus);
		EXPECT_EQ(0u, result.CompletionCode);
	}

	TEST(TEST_CLASS, CanCreateAbortConsumerResult) {
		// Act:
		auto result = Abort(static_cast<validators::ValidationResult>(456));

		// Assert:
		EXPECT_EQ(disruptor::CompletionStatus::Aborted, result.CompletionStatus);
		EXPECT_EQ(456u, result.CompletionCode);
	}

	TEST(TEST_CLASS, CanCreateCompleteConsumerResult) {
		// Act:
		auto result = Complete();

		// Assert:
		EXPECT_EQ(disruptor::CompletionStatus::Consumed, result.CompletionStatus);
		EXPECT_EQ(0u, result.CompletionCode);
	}
}}

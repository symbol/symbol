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

#include "catapult/consumers/ConsumerUtils.h"
#include "catapult/consumers/ConsumerResults.h"
#include "catapult/disruptor/ConsumerInput.h"
#include "catapult/ionet/NodeInteractionResult.h"
#include "catapult/validators/ValidationResult.h"
#include "tests/TestHarness.h"

namespace catapult { namespace consumers {

#define TEST_CLASS ConsumerUtilsTests

	namespace {
		void AssertNodeInteractionResult(ionet::NodeInteractionResultCode expectedCode, validators::ValidationResult validationResult) {
			// Arrange:
			auto identityKey = test::GenerateRandomByteArray<Key>();
			model::AnnotatedTransactionRange range;
			range.SourceIdentity = { identityKey, "11.22.33.44" };
			disruptor::ConsumerInput input(std::move(range));

			disruptor::ConsumerCompletionResult consumerResult;
			consumerResult.CompletionCode = utils::to_underlying_type(validationResult);

			// Act:
			auto result = ToNodeInteractionResult(input.sourceIdentity(), consumerResult);

			// Assert:
			EXPECT_EQ(identityKey, result.Identity.PublicKey);
			EXPECT_EQ("11.22.33.44", result.Identity.Host);
			EXPECT_EQ(expectedCode, result.Code) << "for validation result " << validationResult;
		}
	}

	TEST(TEST_CLASS, NeutralWhenConsumerResultIsNeutral) {
		AssertNodeInteractionResult(ionet::NodeInteractionResultCode::Neutral, validators::ValidationResult::Neutral);
		AssertNodeInteractionResult(ionet::NodeInteractionResultCode::Neutral, consumers::Neutral_Consumer_Hash_In_Recency_Cache);
		AssertNodeInteractionResult(ionet::NodeInteractionResultCode::Neutral, static_cast<validators::ValidationResult>(0x40001234));
		AssertNodeInteractionResult(ionet::NodeInteractionResultCode::Neutral, static_cast<validators::ValidationResult>(0x41234567));
	}

	TEST(TEST_CLASS, SuccessWhenConsumerResultIsSuccess) {
		AssertNodeInteractionResult(ionet::NodeInteractionResultCode::Success, validators::ValidationResult::Success);
		AssertNodeInteractionResult(ionet::NodeInteractionResultCode::Success, static_cast<validators::ValidationResult>(0x00001234));
		AssertNodeInteractionResult(ionet::NodeInteractionResultCode::Success, static_cast<validators::ValidationResult>(0x01234567));
	}

	TEST(TEST_CLASS, FailureWhenConsumerResultIsFailure) {
		AssertNodeInteractionResult(ionet::NodeInteractionResultCode::Failure, validators::ValidationResult::Failure);
		AssertNodeInteractionResult(ionet::NodeInteractionResultCode::Failure, static_cast<validators::ValidationResult>(0x80001234));
		AssertNodeInteractionResult(ionet::NodeInteractionResultCode::Failure, static_cast<validators::ValidationResult>(0x81234567));
	}
}}

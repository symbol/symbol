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

#include "catapult/extensions/NodeInteractionUtils.h"
#include "catapult/ionet/NodeContainer.h"
#include "catapult/ionet/NodeInteractionResult.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS NodeInteractionUtilsTests

	namespace {
		template<typename TAssert>
		void AssertIncrement(ionet::NodeInteractionResultCode code, TAssert assertFunc) {
			// Arrange:
			auto identity = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" };
			ionet::NodeContainer container;
			container.modifier().add(test::CreateNamedNode(identity, "Alice"), ionet::NodeSource::Static);

			// Sanity:
			EXPECT_TRUE(container.view().contains(identity));

			// Act:
			IncrementNodeInteraction(container, ionet::NodeInteractionResult(identity, code));

			// Assert:
			auto interactions = container.view().getNodeInfo(identity).interactions(Timestamp());
			assertFunc(interactions);
		}
	}

	TEST(TEST_CLASS, SuccessCounterIsIncrementedOnSuccessfulInteraction) {
		// Act:
		AssertIncrement(ionet::NodeInteractionResultCode::Success, [](const auto& interactions) {
			EXPECT_EQ(1u, interactions.NumSuccesses);
			EXPECT_EQ(0u, interactions.NumFailures);
		});
	}

	TEST(TEST_CLASS, FailureCounterIsIncrementedOnFailedInteraction) {
		// Act:
		AssertIncrement(ionet::NodeInteractionResultCode::Failure, [](const auto& interactions) {
			EXPECT_EQ(0u, interactions.NumSuccesses);
			EXPECT_EQ(1u, interactions.NumFailures);
		});
	}

	TEST(TEST_CLASS, NoCounterIsIncrementedOnNoneInteraction) {
		// Act:
		AssertIncrement(ionet::NodeInteractionResultCode::None, [](const auto& interactions) {
			EXPECT_EQ(0u, interactions.NumSuccesses);
			EXPECT_EQ(0u, interactions.NumFailures);
		});
	}

	TEST(TEST_CLASS, NoCounterIsIncrementedOnNeutralInteraction) {
		// Act:
		AssertIncrement(ionet::NodeInteractionResultCode::Neutral, [](const auto& interactions) {
			EXPECT_EQ(0u, interactions.NumSuccesses);
			EXPECT_EQ(0u, interactions.NumFailures);
		});
	}
}}

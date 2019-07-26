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

#include "src/state/AccountRestrictionDescriptor.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS AccountRestrictionDescriptorTests

	namespace {
		constexpr auto Outgoing_Address = model::AccountRestrictionType::Address | model::AccountRestrictionType::Outgoing;
	}

	TEST(TEST_CLASS, CanCreateAccountRestrictionDescriptor_Allow) {
		// Act:
		AccountRestrictionDescriptor restrictionDescriptor(model::AccountRestrictionType::Address);

		// Assert:
		EXPECT_EQ(model::AccountRestrictionType::Address, restrictionDescriptor.directionalRestrictionType());
		EXPECT_EQ(model::AccountRestrictionType::Address, restrictionDescriptor.restrictionType());
		EXPECT_EQ(AccountRestrictionOperationType::Allow, restrictionDescriptor.operationType());
		EXPECT_EQ(model::AccountRestrictionType::Address, restrictionDescriptor.raw());
	}

	TEST(TEST_CLASS, CanCreateAccountRestrictionDescriptor_Block) {
		// Act:
		AccountRestrictionDescriptor restrictionDescriptor(model::AccountRestrictionType::Address | model::AccountRestrictionType::Block);

		// Assert:
		EXPECT_EQ(model::AccountRestrictionType::Address, restrictionDescriptor.directionalRestrictionType());
		EXPECT_EQ(model::AccountRestrictionType::Address, restrictionDescriptor.restrictionType());
		EXPECT_EQ(AccountRestrictionOperationType::Block, restrictionDescriptor.operationType());
		EXPECT_EQ(model::AccountRestrictionType::Address | model::AccountRestrictionType::Block, restrictionDescriptor.raw());
	}

	TEST(TEST_CLASS, CanCreateAccountRestrictionDescriptor_OutgoingAllow) {
		// Act:
		AccountRestrictionDescriptor restrictionDescriptor(Outgoing_Address);

		// Assert:
		EXPECT_EQ(Outgoing_Address, restrictionDescriptor.directionalRestrictionType());
		EXPECT_EQ(model::AccountRestrictionType::Address, restrictionDescriptor.restrictionType());
		EXPECT_EQ(AccountRestrictionOperationType::Allow, restrictionDescriptor.operationType());
		EXPECT_EQ(Outgoing_Address, restrictionDescriptor.raw());
	}

	TEST(TEST_CLASS, CanCreateAccountRestrictionDescriptor_OutgoingBlock) {
		// Act:
		AccountRestrictionDescriptor restrictionDescriptor(Outgoing_Address | model::AccountRestrictionType::Block);

		// Assert:
		EXPECT_EQ(Outgoing_Address, restrictionDescriptor.directionalRestrictionType());
		EXPECT_EQ(model::AccountRestrictionType::Address, restrictionDescriptor.restrictionType());
		EXPECT_EQ(AccountRestrictionOperationType::Block, restrictionDescriptor.operationType());
		EXPECT_EQ(Outgoing_Address | model::AccountRestrictionType::Block, restrictionDescriptor.raw());
	}
}}

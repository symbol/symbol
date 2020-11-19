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

#include "src/state/AccountRestrictionDescriptor.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS AccountRestrictionDescriptorTests

	namespace {
		constexpr auto Outgoing_Address = model::AccountRestrictionFlags::Address | model::AccountRestrictionFlags::Outgoing;
	}

	TEST(TEST_CLASS, CanCreateAccountRestrictionDescriptor_Allow) {
		// Act:
		AccountRestrictionDescriptor restrictionDescriptor(model::AccountRestrictionFlags::Address);

		// Assert:
		EXPECT_EQ(model::AccountRestrictionFlags::Address, restrictionDescriptor.directionalRestrictionFlags());
		EXPECT_EQ(model::AccountRestrictionFlags::Address, restrictionDescriptor.restrictionFlags());
		EXPECT_EQ(AccountRestrictionOperationType::Allow, restrictionDescriptor.operationType());
		EXPECT_EQ(model::AccountRestrictionFlags::Address, restrictionDescriptor.raw());
	}

	TEST(TEST_CLASS, CanCreateAccountRestrictionDescriptor_Block) {
		// Act:
		auto restrictionFlags = model::AccountRestrictionFlags::Address | model::AccountRestrictionFlags::Block;
		AccountRestrictionDescriptor restrictionDescriptor(restrictionFlags);

		// Assert:
		EXPECT_EQ(model::AccountRestrictionFlags::Address, restrictionDescriptor.directionalRestrictionFlags());
		EXPECT_EQ(model::AccountRestrictionFlags::Address, restrictionDescriptor.restrictionFlags());
		EXPECT_EQ(AccountRestrictionOperationType::Block, restrictionDescriptor.operationType());
		EXPECT_EQ(model::AccountRestrictionFlags::Address | model::AccountRestrictionFlags::Block, restrictionDescriptor.raw());
	}

	TEST(TEST_CLASS, CanCreateAccountRestrictionDescriptor_OutgoingAllow) {
		// Act:
		AccountRestrictionDescriptor restrictionDescriptor(Outgoing_Address);

		// Assert:
		EXPECT_EQ(Outgoing_Address, restrictionDescriptor.directionalRestrictionFlags());
		EXPECT_EQ(model::AccountRestrictionFlags::Address, restrictionDescriptor.restrictionFlags());
		EXPECT_EQ(AccountRestrictionOperationType::Allow, restrictionDescriptor.operationType());
		EXPECT_EQ(Outgoing_Address, restrictionDescriptor.raw());
	}

	TEST(TEST_CLASS, CanCreateAccountRestrictionDescriptor_OutgoingBlock) {
		// Act:
		AccountRestrictionDescriptor restrictionDescriptor(Outgoing_Address | model::AccountRestrictionFlags::Block);

		// Assert:
		EXPECT_EQ(Outgoing_Address, restrictionDescriptor.directionalRestrictionFlags());
		EXPECT_EQ(model::AccountRestrictionFlags::Address, restrictionDescriptor.restrictionFlags());
		EXPECT_EQ(AccountRestrictionOperationType::Block, restrictionDescriptor.operationType());
		EXPECT_EQ(Outgoing_Address | model::AccountRestrictionFlags::Block, restrictionDescriptor.raw());
	}
}}

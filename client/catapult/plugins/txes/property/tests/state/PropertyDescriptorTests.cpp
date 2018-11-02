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

#include "src/state/PropertyDescriptor.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS PropertyDescriptorTests

	TEST(TEST_CLASS, CanCreatePropertyDescriptor_Allow) {
		// Act:
		PropertyDescriptor propertyDescriptor(model::PropertyType::Address);

		// Assert:
		EXPECT_EQ(model::PropertyType::Address, propertyDescriptor.propertyType());
		EXPECT_EQ(OperationType::Allow, propertyDescriptor.operationType());
		EXPECT_EQ(model::PropertyType::Address, propertyDescriptor.raw());
	}

	TEST(TEST_CLASS, CanCreatePropertyDescriptor_Block) {
		// Act:
		PropertyDescriptor propertyDescriptor(model::PropertyType::Address | model::PropertyType::Block);

		// Assert:
		EXPECT_EQ(model::PropertyType::Address, propertyDescriptor.propertyType());
		EXPECT_EQ(OperationType::Block, propertyDescriptor.operationType());
		EXPECT_EQ(model::PropertyType::Address | model::PropertyType::Block, propertyDescriptor.raw());
	}
}}

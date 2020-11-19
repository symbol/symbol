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

#include "src/state/NamespaceAlias.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS NamespaceAliasTests

	namespace {
		void AssertMosaicAlias(const NamespaceAlias& alias, MosaicId mosaicId) {
			// Assert:
			EXPECT_EQ(AliasType::Mosaic, alias.type());
			EXPECT_EQ(mosaicId, alias.mosaicId());
			EXPECT_THROW(alias.address(), catapult_runtime_error);
		}

		void AssertAddressAlias(const NamespaceAlias& alias, const Address& address) {
			// Assert:
			EXPECT_EQ(AliasType::Address, alias.type());
			EXPECT_THROW(alias.mosaicId(), catapult_runtime_error);
			EXPECT_EQ(address, alias.address());
		}
	}

	// region constructor

	TEST(TEST_CLASS, CanCreateUnsetNamespaceAlias) {
		// Act:
		NamespaceAlias alias;

		// Assert:
		EXPECT_EQ(AliasType::None, alias.type());
		EXPECT_THROW(alias.mosaicId(), catapult_runtime_error);
		EXPECT_THROW(alias.address(), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanCreateMosaicNamespaceAlias) {
		// Act:
		NamespaceAlias alias(MosaicId(245));

		// Assert:
		AssertMosaicAlias(alias, MosaicId(245));
	}

	TEST(TEST_CLASS, CanCreateAddressNamespaceAlias) {
		// Act:
		auto address = test::GenerateRandomByteArray<Address>();
		NamespaceAlias alias(address);

		// Assert:
		AssertAddressAlias(alias, address);
	}

	// endregion

	// region copy constructor

	TEST(TEST_CLASS, CanCopyMosaicNamespaceAlias) {
		// Arrange:
		NamespaceAlias originalAlias(MosaicId(245));

		// Act:
		NamespaceAlias alias(originalAlias);

		// Assert:
		AssertMosaicAlias(alias, MosaicId(245));
	}

	TEST(TEST_CLASS, CanCopyAddressNamespaceAlias) {
		// Arrange:
		auto address = test::GenerateRandomByteArray<Address>();
		NamespaceAlias originalAlias(address);

		// Act:
		NamespaceAlias alias(originalAlias);

		// Assert:
		AssertAddressAlias(alias, address);
	}

	// endregion

	// region assignment operator

	TEST(TEST_CLASS, CanAssignMosaicNamespaceAlias) {
		// Arrange:
		NamespaceAlias originalAlias(MosaicId(245));

		// Act:
		NamespaceAlias alias;
		const auto& result = (alias = originalAlias);

		// Assert:
		EXPECT_EQ(&alias, &result);
		AssertMosaicAlias(alias, MosaicId(245));
	}

	TEST(TEST_CLASS, CanAssignAddressNamespaceAlias) {
		// Arrange:
		auto address = test::GenerateRandomByteArray<Address>();
		NamespaceAlias originalAlias(address);

		// Act:
		NamespaceAlias alias;
		const auto& result = (alias = originalAlias);

		// Assert:
		EXPECT_EQ(&alias, &result);
		AssertAddressAlias(alias, address);
	}

	// endregion
}}

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

#include "MultisigMapperTestUtils.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	namespace {
		void AssertAddressSet(const state::SortedAddressSet& addresses, const bsoncxx::document::view& dbAddresses) {
			ASSERT_EQ(addresses.size(), test::GetFieldCount(dbAddresses));

			for (auto dbIter = dbAddresses.cbegin(); dbAddresses.cend() != dbIter; ++dbIter) {
				auto address = test::GetByteArrayFromMongoSource<Address>(*dbIter);
				EXPECT_CONTAINS(addresses, address);
			}
		}
	}

	void AssertEqualMultisigData(const state::MultisigEntry& entry, const bsoncxx::document::view& dbMultisig) {

		EXPECT_EQ(entry.address(), GetAddressValue(dbMultisig, "accountAddress"));
		EXPECT_EQ(entry.minApproval(), GetUint32(dbMultisig, "minApproval"));
		EXPECT_EQ(entry.minRemoval(), GetUint32(dbMultisig, "minRemoval"));

		AssertAddressSet(entry.cosignatoryAddresses(), dbMultisig["cosignatoryAddresses"].get_array().value);
		AssertAddressSet(entry.multisigAddresses(), dbMultisig["multisigAddresses"].get_array().value);
	}
}}

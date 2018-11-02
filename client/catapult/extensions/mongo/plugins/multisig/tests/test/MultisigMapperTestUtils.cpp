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

	// region multisig entry related

	namespace {
		void AssertKeySet(const utils::SortedKeySet& keySet, const bsoncxx::document::view& dbKeySet) {
			ASSERT_EQ(keySet.size(), test::GetFieldCount(dbKeySet));

			for (auto dbIter = dbKeySet.cbegin(); dbKeySet.cend() != dbIter; ++dbIter) {
				Key key;
				mongo::mappers::DbBinaryToModelArray(key, dbIter->get_binary());
				EXPECT_TRUE(keySet.cend() != keySet.find(key)) << "for public key " << utils::HexFormat(key);
			}
		}
	}

	void AssertEqualMultisigData(const state::MultisigEntry& entry, const Address& address, const bsoncxx::document::view& dbMultisig) {
		EXPECT_EQ(address, test::GetAddressValue(dbMultisig, "accountAddress"));

		EXPECT_EQ(entry.key(), GetKeyValue(dbMultisig, "account"));
		EXPECT_EQ(entry.minApproval(), GetUint32(dbMultisig, "minApproval"));
		EXPECT_EQ(entry.minRemoval(), GetUint32(dbMultisig, "minRemoval"));

		AssertKeySet(entry.cosignatories(), dbMultisig["cosignatories"].get_array().value);
		AssertKeySet(entry.multisigAccounts(), dbMultisig["multisigAccounts"].get_array().value);
	}

	// endregion
}}

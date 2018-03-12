#include "MultisigMapperTestUtils.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	// region multisig entry related

	namespace {
		void AssertKeySet(const utils::KeySet& keySet, const bsoncxx::document::view& dbKeySet) {
			ASSERT_EQ(keySet.size(), std::distance(dbKeySet.cbegin(), dbKeySet.cend()));

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

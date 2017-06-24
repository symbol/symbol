#include "MapperTestUtils.h"
#include "plugins/mongo/coremongo/src/mappers/MapperUtils.h"
#include "plugins/txes/multisig/src/state/MultisigEntry.h"
#include "tests/test/mongo/MapperTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::test;

namespace catapult { namespace mongo { namespace test {

	// region multisig entry related

	namespace {
		void AssertKeySet(const utils::KeySet& keySet, const bsoncxx::document::view& dbKeySet) {
			ASSERT_EQ(keySet.size(), std::distance(dbKeySet.cbegin(), dbKeySet.cend()));

			for (auto dbIter = dbKeySet.cbegin(); dbKeySet.cend() != dbIter; ++dbIter) {
				Key key;
				mappers::DbBinaryToModelArray(key, dbIter->get_binary());
				EXPECT_TRUE(keySet.cend() != keySet.find(key)) << "for public key " << ToHexString(key);
			}
		}
	}

	void AssertEqualMultisigData(const state::MultisigEntry& entry, const bsoncxx::document::view& dbMultisig) {
		EXPECT_EQ(ToHexString(entry.key()), ToHexString(GetBinary(dbMultisig, "account"), Key_Size));
		EXPECT_EQ(entry.minApproval(), GetUint32(dbMultisig, "minApproval"));
		EXPECT_EQ(entry.minRemoval(), GetUint32(dbMultisig, "minRemoval"));

		AssertKeySet(entry.cosignatories(), dbMultisig["cosignatories"].get_array().value);
		AssertKeySet(entry.multisigAccounts(), dbMultisig["multisigAccounts"].get_array().value);
	}

	// endregion
}}}

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

#pragma once
#include "src/model/MultisigAccountModificationTransaction.h"
#include "src/model/MultisigNotifications.h"
#include "catapult/model/Cosignature.h"
#include "catapult/utils/HexFormatter.h"
#include "tests/TestHarness.h"

namespace catapult {
	namespace cache { class CatapultCacheDelta; }
	namespace state { class MultisigEntry; }
}

namespace catapult { namespace test {

	/// Asserts that all \a expectedKeys are contained within \a keys.
	template<typename TExpectedKeys, typename TKeys>
	void AssertContents(const TExpectedKeys& expectedKeys, const TKeys& keys) {
		// Assert:
		EXPECT_EQ(expectedKeys.size(), keys.size());
		for (const auto& key : expectedKeys)
			EXPECT_CONTAINS(keys, key);
	}

	/// Asserts that all \a expectedKeys are contained within \a cache.
	template<typename TExpectedKeys, typename TCache>
	void AssertMultisigCacheContents(const TExpectedKeys& expectedKeys, const TCache& cache) {
		// Assert:
		EXPECT_EQ(expectedKeys.size(), cache.size());
		for (const auto& key : expectedKeys)
			EXPECT_TRUE(cache.contains(key)) << utils::HexFormat(key[0]);
	}

	/// Generates \a count random keys.
	std::vector<Key> GenerateKeys(size_t count);

	/// Generates random cosignatures from \a cosignatories.
	std::vector<model::Cosignature> GenerateCosignaturesFromCosignatories(const std::vector<Key>& cosignatories);

	/// Creates a multisig account modification transaction from \a signer with \a numAdditions additions and \a numDeletions deletions.
	std::unique_ptr<model::EmbeddedMultisigAccountModificationTransaction> CreateMultisigAccountModificationTransaction(
			const Key& signer,
			uint8_t numAdditions,
			uint8_t numDeletions);

	/// Creates a multisig cosignatories notification around \a signer, \a publicKeyAdditions and \a publicKeyDeletions.
	model::MultisigCosignatoriesNotification CreateMultisigCosignatoriesNotification(
			const Key& signer,
			const std::vector<Key>& publicKeyAdditions,
			const std::vector<Key>& publicKeyDeletions);

	/// Makes \a multisigKey in \a cache a multisig account with \a cosignatoryKeys as cosignatories and required limits
	/// \a minApproval and \a minRemoval.
	void MakeMultisig(
			cache::CatapultCacheDelta& cache,
			const Key& multisigKey,
			const std::vector<Key>& cosignatoryKeys,
			uint32_t minApproval = 0,
			uint32_t minRemoval = 0);

	/// Asserts that multisig entry \a actual is equal to \a expected.
	void AssertEqual(const state::MultisigEntry& expected, const state::MultisigEntry& actual);
}}

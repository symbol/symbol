#pragma once
#include "src/model/ModifyMultisigAccountTransaction.h"
#include "catapult/model/Cosignature.h"
#include "catapult/utils/HexFormatter.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache { class CatapultCacheDelta; } }

namespace catapult { namespace test {

	/// Asserts that all \a expectedKeys are contained within \a keys.
	template<typename TExpectedKeys, typename TKeys>
	void AssertContents(const TExpectedKeys& expectedKeys, const TKeys& keys) {
		// Assert:
		EXPECT_EQ(expectedKeys.size(), keys.size());
		for (const auto& key : expectedKeys)
			EXPECT_TRUE(keys.cend() != keys.find(key)) << utils::HexFormat(key[0]);
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

	/// Generates random cosignatures from \a cosigners.
	std::vector<model::Cosignature> GenerateCosignaturesFromCosigners(const std::vector<Key>& cosigners);

	/// Creates a modify multisig account transaction from \a signer with \a modificationTypes.
	std::unique_ptr<model::EmbeddedModifyMultisigAccountTransaction> CreateModifyMultisigAccountTransaction(
			const Key& signer,
			const std::vector<model::CosignatoryModificationType>& modificationTypes);

	/// Makes \a multisigKey in \a cache a multisig account with \a cosignatoryKeys as cosignatories and required limits
	/// \a minApproval and \a minRemoval.
	void MakeMultisig(
			cache::CatapultCacheDelta& cache,
			const Key& multisigKey,
			const std::vector<Key>& cosignatoryKeys,
			uint8_t minApproval = 0,
			uint8_t minRemoval = 0);
}}

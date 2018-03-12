#include "MultisigTestUtils.h"
#include "src/cache/MultisigCache.h"
#include "catapult/cache/CatapultCacheDelta.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/nodeps/Random.h"

namespace catapult { namespace test {

	std::vector<Key> GenerateKeys(size_t count) {
		return test::GenerateRandomDataVector<Key>(count);
	}

	std::vector<model::Cosignature> GenerateCosignaturesFromCosigners(const std::vector<Key>& cosigners) {
		auto cosignatures = test::GenerateRandomDataVector<model::Cosignature>(cosigners.size());
		for (auto i = 0u; i < cosigners.size(); ++i)
			cosignatures[i].Signer = cosigners[i];

		return cosignatures;
	}

	std::unique_ptr<model::EmbeddedModifyMultisigAccountTransaction> CreateModifyMultisigAccountTransaction(
			const Key& signer,
			const std::vector<model::CosignatoryModificationType>& modificationTypes) {
		using TransactionType = model::EmbeddedModifyMultisigAccountTransaction;
		auto numModifications = static_cast<uint8_t>(modificationTypes.size());
		uint32_t entitySize = sizeof(TransactionType) + numModifications * sizeof(model::CosignatoryModification);
		auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
		pTransaction->Size = entitySize;
		pTransaction->ModificationsCount = numModifications;
		pTransaction->Type = model::Entity_Type_Modify_Multisig_Account;
		pTransaction->Signer = signer;

		auto* pModification = pTransaction->ModificationsPtr();
		for (auto i = 0u; i < numModifications; ++i) {
			pModification->ModificationType = modificationTypes[i];
			test::FillWithRandomData(pModification->CosignatoryPublicKey);
			++pModification;
		}

		return pTransaction;
	}

	namespace {
		state::MultisigEntry& GetOrCreateEntry(cache::MultisigCacheDelta& multisigCache, const Key& key) {
			if (!multisigCache.contains(key))
				multisigCache.insert(state::MultisigEntry(key));

			return multisigCache.get(key);
		}
	}

	void MakeMultisig(
			cache::CatapultCacheDelta& cache,
			const Key& multisigKey,
			const std::vector<Key>& cosignatoryKeys,
			uint8_t minApproval,
			uint8_t minRemoval) {
		auto& multisigCache = cache.sub<cache::MultisigCache>();

		auto& multisigEntry = GetOrCreateEntry(multisigCache, multisigKey);
		multisigEntry.setMinApproval(minApproval);
		multisigEntry.setMinRemoval(minRemoval);

		// add all cosignatories
		for (const auto& cosignatoryKey : cosignatoryKeys) {
			multisigEntry.cosignatories().insert(cosignatoryKey);

			auto& cosignatoryEntry = GetOrCreateEntry(multisigCache, cosignatoryKey);
			cosignatoryEntry.multisigAccounts().insert(multisigKey);
		}
	}
}}

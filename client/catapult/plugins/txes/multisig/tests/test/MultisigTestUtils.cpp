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

#include "MultisigTestUtils.h"
#include "src/cache/MultisigCache.h"
#include "catapult/cache/CatapultCacheDelta.h"
#include "catapult/model/Address.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/nodeps/Random.h"

namespace catapult { namespace test {

	std::vector<model::Cosignature> GenerateCosignaturesFromCosignatories(const std::vector<Key>& cosignatories) {
		auto cosignatures = GenerateRandomDataVector<model::Cosignature>(cosignatories.size());
		for (auto i = 0u; i < cosignatories.size(); ++i)
			cosignatures[i].SignerPublicKey = cosignatories[i];

		return cosignatures;
	}

	std::unique_ptr<model::EmbeddedMultisigAccountModificationTransaction> CreateMultisigAccountModificationTransaction(
			const Key& signer,
			uint8_t numAdditions,
			uint8_t numDeletions) {
		return CreateMultisigAccountModificationTransaction(
				signer,
				GenerateRandomDataVector<UnresolvedAddress>(numAdditions),
				GenerateRandomDataVector<UnresolvedAddress>(numDeletions));
	}

	std::unique_ptr<model::EmbeddedMultisigAccountModificationTransaction> CreateMultisigAccountModificationTransaction(
			const Key& signer,
			const std::vector<UnresolvedAddress>& addressAdditions,
			const std::vector<UnresolvedAddress>& addressDeletions) {
		using TransactionType = model::EmbeddedMultisigAccountModificationTransaction;
		uint32_t entitySize = sizeof(TransactionType);
		entitySize += static_cast<uint32_t>((addressAdditions.size() + addressDeletions.size()) * UnresolvedAddress::Size);

		auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
		FillWithRandomData({ reinterpret_cast<uint8_t*>(pTransaction.get()), entitySize });

		pTransaction->Size = entitySize;
		pTransaction->AddressAdditionsCount = static_cast<uint8_t>(addressAdditions.size());
		pTransaction->AddressDeletionsCount = static_cast<uint8_t>(addressDeletions.size());
		pTransaction->Type = model::Entity_Type_Multisig_Account_Modification;
		pTransaction->SignerPublicKey = signer;

		std::copy(addressAdditions.cbegin(), addressAdditions.cend(), pTransaction->AddressAdditionsPtr());
		std::copy(addressDeletions.cbegin(), addressDeletions.cend(), pTransaction->AddressDeletionsPtr());
		return pTransaction;
	}

	model::MultisigCosignatoriesNotification CreateMultisigCosignatoriesNotification(
			const Address& multisig,
			const std::vector<UnresolvedAddress>& addressAdditions,
			const std::vector<UnresolvedAddress>& addressDeletions) {
		return model::MultisigCosignatoriesNotification(
				multisig,
				static_cast<uint8_t>(addressAdditions.size()),
				addressAdditions.data(),
				static_cast<uint8_t>(addressDeletions.size()),
				addressDeletions.data());
	}

	namespace {
		state::MultisigEntry& GetOrCreateEntry(cache::MultisigCacheDelta& multisigCache, const Address& address) {
			if (!multisigCache.contains(address))
				multisigCache.insert(state::MultisigEntry(address));

			return multisigCache.find(address).get();
		}
	}

	void MakeMultisig(
			cache::CatapultCacheDelta& cache,
			const Address& multisig,
			const std::vector<Address>& cosignatories,
			uint32_t minApproval,
			uint32_t minRemoval) {
		auto& multisigCache = cache.sub<cache::MultisigCache>();

		auto& multisigEntry = GetOrCreateEntry(multisigCache, multisig);
		multisigEntry.setMinApproval(minApproval);
		multisigEntry.setMinRemoval(minRemoval);

		// add all cosignatories
		for (const auto& cosignatory : cosignatories) {
			multisigEntry.cosignatoryAddresses().insert(cosignatory);

			auto& cosignatoryEntry = GetOrCreateEntry(multisigCache, cosignatory);
			cosignatoryEntry.multisigAddresses().insert(multisig);
		}
	}

	namespace {
		void AssertEqual(const state::SortedAddressSet& expected, const state::SortedAddressSet& actual) {
			ASSERT_EQ(expected.size(), actual.size());
			EXPECT_EQ(expected, actual);
		}
	}

	void AssertEqual(const state::MultisigEntry& expectedEntry, const state::MultisigEntry& entry) {
		EXPECT_EQ(expectedEntry.minApproval(), entry.minApproval());
		EXPECT_EQ(expectedEntry.minRemoval(), entry.minRemoval());

		EXPECT_EQ(expectedEntry.address(), entry.address());

		AssertEqual(expectedEntry.cosignatoryAddresses(), entry.cosignatoryAddresses());
		AssertEqual(expectedEntry.multisigAddresses(), entry.multisigAddresses());
	}
}}

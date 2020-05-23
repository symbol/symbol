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
#include "catapult/model/Address.h"
#include "catapult/model/Cosignature.h"
#include "catapult/utils/HexFormatter.h"
#include "tests/test/core/ResolverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult {
	namespace cache { class CatapultCacheDelta; }
	namespace state { class MultisigEntry; }
}

namespace catapult { namespace test {

	/// Asserts that all \a expectedAddresses are contained within \a addresses.
	template<typename TExpectedAddresses, typename TAddresses>
	void AssertContents(const TExpectedAddresses& expectedAddresses, const TAddresses& addresses) {
		// Assert:
		EXPECT_EQ(expectedAddresses.size(), addresses.size());
		for (const auto& address : expectedAddresses)
			EXPECT_CONTAINS(addresses, address);
	}

	/// Functions for performing network address conversions.
	template<model::NetworkIdentifier Network_Identifier = model::NetworkIdentifier::Zero>
	struct NetworkAddressConversions {
	public:
		/// Converts \a publicKey to an address.
		static Address ToAddress(const Key& publicKey) {
			return model::PublicKeyToAddress(publicKey, Network_Identifier);
		}

		/// Converts \a publicKeys to (resolved) addresses.
		static std::vector<Address> ToAddresses(const std::vector<Key>& publicKeys) {
			std::vector<Address> addresses;
			for (const auto& publicKey : publicKeys)
				addresses.push_back(ToAddress(publicKey));

			return addresses;
		}

		/// Converts \a publicKeys to (unresolved) addresses.
		static std::vector<UnresolvedAddress> ToUnresolvedAddresses(const std::vector<Key>& publicKeys) {
			std::vector<UnresolvedAddress> addresses;
			for (const auto& publicKey : publicKeys)
				addresses.push_back(UnresolveXor(ToAddress(publicKey)));

			return addresses;
		}
	};

	/// Generates random cosignatures from \a cosignatories.
	std::vector<model::Cosignature> GenerateCosignaturesFromCosignatories(const std::vector<Key>& cosignatories);

	/// Creates a multisig account modification transaction from \a signer with \a numAdditions additions and \a numDeletions deletions.
	std::unique_ptr<model::EmbeddedMultisigAccountModificationTransaction> CreateMultisigAccountModificationTransaction(
			const Key& signer,
			uint8_t numAdditions,
			uint8_t numDeletions);

	/// Creates a multisig account modification transaction from \a signer with \a addressAdditions and \a addressDeletions.
	std::unique_ptr<model::EmbeddedMultisigAccountModificationTransaction> CreateMultisigAccountModificationTransaction(
			const Key& signer,
			const std::vector<UnresolvedAddress>& addressAdditions,
			const std::vector<UnresolvedAddress>& addressDeletions);

	/// Creates a multisig cosignatories notification around \a multisig, \a addressAdditions and \a addressDeletions.
	model::MultisigCosignatoriesNotification CreateMultisigCosignatoriesNotification(
			const Address& multisig,
			const std::vector<UnresolvedAddress>& addressAdditions,
			const std::vector<UnresolvedAddress>& addressDeletions);

	/// Makes \a multisig in \a cache a multisig account with \a cosignatories and required limits
	/// \a minApproval and \a minRemoval.
	void MakeMultisig(
			cache::CatapultCacheDelta& cache,
			const Address& multisig,
			const std::vector<Address>& cosignatories,
			uint32_t minApproval = 0,
			uint32_t minRemoval = 0);

	/// Asserts that multisig entry \a actual is equal to \a expected.
	void AssertEqual(const state::MultisigEntry& expected, const state::MultisigEntry& actual);
}}

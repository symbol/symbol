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
#include "EntityBody.h"
#include "SizePrefixedEntity.h"
#include <iosfwd>

namespace catapult {
	namespace model {
		class NotificationSubscriber;
		class TransactionRegistry;
	}
}

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for an embedded transaction header.
	struct EmbeddedTransactionHeader : public SizePrefixedEntity {
	public:
		/// Size of the header that can be skipped when signing/verifying.
		/// \note This skips EmbeddedTransactionHeader and beginning of EntityBody.
		static constexpr size_t Header_Size = sizeof(SizePrefixedEntity) + 2 * sizeof(uint32_t) + Key::Size;

	public:
		/// Reserved padding to align end of EmbeddedTransactionHeader on 8-byte boundary.
		uint32_t EmbeddedTransactionHeader_Reserved1;
	};

	/// Binary layout for an embedded transaction (non-verifiable).
	struct PLUGIN_API_DEPENDENCY EmbeddedTransaction : public EntityBody<EmbeddedTransactionHeader> {};

	// EmbeddedTransaction Layout:
	// * SizePrefixedEntity
	//   0x00:  (4) Size
	// * EmbeddedTransaction
	//   0x04:  (4) EmbeddedTransactionHeader_Reserved1
	// * EntityBody
	//   0x08: (32) SignerPublicKey
	//   0x28:  (4) EntityBody_Reserved1
	//   0x2C:  (1) Version
	//   0x2D:  (1) Network
	//   0x2E:  (2) Type
	// * EmbeddedTransaction
	//   0x30:  (*) Transaction Data

#pragma pack(pop)

	/// Insertion operator for outputting \a transaction to \a out.
	std::ostream& operator<<(std::ostream& out, const EmbeddedTransaction& transaction);

	/// Gets the address of the signer of \a transaction.
	Address GetSignerAddress(const EmbeddedTransaction& transaction);

	/// Checks the real size of \a transaction against its reported size and returns \c true if the sizes match.
	/// \a registry contains all known transaction types.
	bool IsSizeValid(const EmbeddedTransaction& transaction, const TransactionRegistry& registry);

	/// Sends all notifications from \a transaction to \a sub.
	void PublishNotifications(const EmbeddedTransaction& transaction, NotificationSubscriber& sub);

	/// Advances to the next embedded transaction following padded \a pTransaction.
	const model::EmbeddedTransaction* AdvanceNext(const model::EmbeddedTransaction* pTransaction);
}}

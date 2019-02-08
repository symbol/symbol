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
#include "ReceiptType.h"
#include "SizePrefixedEntity.h"
#include "catapult/types.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a receipt entity.
	struct Receipt : public SizePrefixedEntity {
		/// Receipt version.
		uint16_t Version;

		/// Receipt type.
		ReceiptType Type;
	};

	/// Binary layout for a balance transfer receipt.
	struct BalanceTransferReceipt : public Receipt {
	public:
		/// Creates a receipt around \a receiptType, \a sender, \a recipient, \a mosaicId and \a amount.
		BalanceTransferReceipt(
				ReceiptType receiptType,
				const Key& sender,
				const Address& recipient,
				catapult::MosaicId mosaicId,
				catapult::Amount amount)
				: Sender(sender)
				, Recipient(recipient)
				, MosaicId(mosaicId)
				, Amount(amount) {
			Size = sizeof(BalanceTransferReceipt);
			Version = 1;
			Type = receiptType;
		}

	public:
		/// Mosaic sender public key.
		Key Sender;

		/// Mosaic recipient address.
		Address Recipient;

		/// Mosaic id.
		catapult::MosaicId MosaicId;

		/// Amount.
		catapult::Amount Amount;
	};

	/// Binary layout for a balance change receipt.
	struct BalanceChangeReceipt : public Receipt {
	public:
		/// Creates a receipt around \a receiptType, \a account, \a mosaicId and \a amount.
		BalanceChangeReceipt(ReceiptType receiptType, const Key& account, catapult::MosaicId mosaicId, catapult::Amount amount)
				: Account(account)
				, MosaicId(mosaicId)
				, Amount(amount) {
			Size = sizeof(BalanceChangeReceipt);
			Version = 1;
			Type = receiptType;
		}

	public:
		/// Account public key.
		Key Account;

		/// Mosaic id.
		catapult::MosaicId MosaicId;

		/// Amount.
		catapult::Amount Amount;
	};

	/// Binary layout for an artifact expiry receipt.
	template<typename TArtifactId>
	struct ArtifactExpiryReceipt : public Receipt {
	public:
		/// Creates a receipt around \a receiptType and \a artifactId.
		ArtifactExpiryReceipt(ReceiptType receiptType, TArtifactId artifactId) : ArtifactId(artifactId) {
			Size = sizeof(ArtifactExpiryReceipt);
			Version = 1;
			Type = receiptType;
		}

	public:
		/// Artifact id.
		TArtifactId ArtifactId;
	};

#pragma pack(pop)

/// Defines constants for a receipt with \a TYPE and \a VERSION.
#define DEFINE_RECEIPT_CONSTANTS(TYPE, VERSION) \
	/* Receipt format version. */ \
	static constexpr uint8_t Current_Version = VERSION; \
	/* Receipt type. */ \
	static constexpr ReceiptType Receipt_Type = TYPE;
}}

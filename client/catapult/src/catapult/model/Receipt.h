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
#include "Mosaic.h"
#include "ReceiptType.h"
#include "SizePrefixedEntity.h"
#include "catapult/types.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	// region Receipt

	/// Binary layout for a receipt entity.
	struct Receipt : public SizePrefixedEntity {
		/// Receipt version.
		uint16_t Version;

		/// Receipt type.
		ReceiptType Type;
	};

	// endregion

	// region BalanceTransferReceipt

	/// Binary layout for a balance transfer receipt.
	struct BalanceTransferReceipt : public Receipt {
	public:
		/// Creates a receipt around \a receiptType, \a senderAddress, \a recipientAddress, \a mosaicId and \a amount.
		BalanceTransferReceipt(
				ReceiptType receiptType,
				const Address& senderAddress,
				const Address& recipientAddress,
				catapult::MosaicId mosaicId,
				catapult::Amount amount)
				: Mosaic({ mosaicId, amount })
				, SenderAddress(senderAddress)
				, RecipientAddress(recipientAddress) {
			Size = sizeof(BalanceTransferReceipt);
			Version = 1;
			Type = receiptType;
		}

	public:
		/// Mosaic.
		model::Mosaic Mosaic;

		/// Mosaic sender address.
		Address SenderAddress;

		/// Mosaic recipient address.
		Address RecipientAddress;
	};

	// endregion

	// region BalanceChangeReceipt

	/// Binary layout for a balance change receipt.
	struct BalanceChangeReceipt : public Receipt {
	public:
		/// Creates a receipt around \a receiptType, \a targetAddress, \a mosaicId and \a amount.
		BalanceChangeReceipt(ReceiptType receiptType, const Address& targetAddress, catapult::MosaicId mosaicId, catapult::Amount amount)
				: Mosaic({ mosaicId, amount })
				, TargetAddress(targetAddress) {
			Size = sizeof(BalanceChangeReceipt);
			Version = 1;
			Type = receiptType;
		}

	public:
		/// Mosaic.
		model::Mosaic Mosaic;

		/// Account address.
		Address TargetAddress;
	};

	// endregion

	// region InflationReceipt

	/// Binary layout for an inflation receipt.
	struct InflationReceipt : public Receipt {
	public:
		/// Creates a receipt around \a receiptType, \a mosaicId and \a amount.
		InflationReceipt(ReceiptType receiptType, catapult::MosaicId mosaicId, catapult::Amount amount) : Mosaic({ mosaicId, amount }) {
			Size = sizeof(InflationReceipt);
			Version = 1;
			Type = receiptType;
		}

	public:
		/// Mosaic.
		model::Mosaic Mosaic;
	};

	// endregion

	// region ArtifactExpiryReceipt

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

	// endregion

#pragma pack(pop)

/// Defines constants for a receipt with \a TYPE and \a VERSION.
#define DEFINE_RECEIPT_CONSTANTS(TYPE, VERSION) \
	/* Receipt format version. */ \
	static constexpr uint8_t Current_Version = VERSION; \
	/* Receipt type. */ \
	static constexpr ReceiptType Receipt_Type = TYPE;
}}

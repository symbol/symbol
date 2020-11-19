/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "FacilityCode.h"
#include "catapult/utils/Casting.h"
#include <iosfwd>
#include <stdint.h>

namespace catapult { namespace model {

	/// Enumeration of basic receipt types.
	/// \note BasicReceiptType is used as highest nibble of receipt type.
	enum class BasicReceiptType : uint8_t {
		/// Some other receipt type.
		Other = 0x0,

		/// Balance transfer.
		BalanceTransfer = 0x1,

		/// Balance credit.
		BalanceCredit = 0x2,

		/// Balance debit.
		BalanceDebit = 0x3,

		/// Artifact expiry receipt.
		ArtifactExpiry = 0x4,

		/// Inflation.
		Inflation = 0x5,

		/// Aggregate receipt.
		Aggregate = 0xE,

		/// Alias resolution.
		AliasResolution = 0xF
	};

	/// Enumeration of receipt types.
	enum class ReceiptType : uint16_t {};

	/// Makes receipt type given \a basicReceiptType, \a facilityCode and \a code.
	constexpr ReceiptType MakeReceiptType(BasicReceiptType basicReceiptType, FacilityCode facilityCode, uint8_t code) {
		return static_cast<ReceiptType>(
				((utils::to_underlying_type(basicReceiptType) & 0x0F) << 12) // 01..04: basic type
				| ((code & 0xF) << 8) //                                        05..08: code
				| (static_cast<uint8_t>(facilityCode) & 0xFF)); //              09..16: facility
	}

/// Defines receipt type given \a BASIC_TYPE, \a FACILITY, \a DESCRIPTION and \a CODE.
#define DEFINE_RECEIPT_TYPE(BASIC_TYPE, FACILITY, DESCRIPTION, CODE) \
	constexpr auto Receipt_Type_##DESCRIPTION = model::MakeReceiptType( \
			(model::BasicReceiptType::BASIC_TYPE), \
			(model::FacilityCode::FACILITY), \
			CODE)

	/// Harvest fee credit.
	DEFINE_RECEIPT_TYPE(BalanceCredit, Core, Harvest_Fee, 1);

	/// Inflation.
	DEFINE_RECEIPT_TYPE(Inflation, Core, Inflation, 1);

	/// Transaction group.
	DEFINE_RECEIPT_TYPE(Aggregate, Core, Transaction_Group, 1);

	/// Address alias resolution.
	DEFINE_RECEIPT_TYPE(AliasResolution, Core, Address_Alias_Resolution, 1);

	/// Mosaic alias resolution.
	DEFINE_RECEIPT_TYPE(AliasResolution, Core, Mosaic_Alias_Resolution, 2);

	/// Insertion operator for outputting \a receiptType to \a out.
	std::ostream& operator<<(std::ostream& out, ReceiptType receiptType);
}}

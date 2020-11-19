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
#include "MosaicRestrictionEntityType.h"
#include "MosaicRestrictionTypes.h"
#include "catapult/model/Transaction.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a mosaic global restriction transaction body.
	template<typename THeader>
	struct MosaicGlobalRestrictionTransactionBody : public THeader {
	private:
		using TransactionType = MosaicGlobalRestrictionTransactionBody<THeader>;

	public:
		DEFINE_TRANSACTION_CONSTANTS(Entity_Type_Mosaic_Global_Restriction, 1)

	public:
		/// Identifier of the mosaic being restricted.
		UnresolvedMosaicId MosaicId;

		/// Identifier of the mosaic providing the restriction key.
		UnresolvedMosaicId ReferenceMosaicId;

		/// Restriction key relative to the reference mosaic identifier.
		uint64_t RestrictionKey;

		/// Previous restriction value.
		uint64_t PreviousRestrictionValue;

		/// New restriction value.
		uint64_t NewRestrictionValue;

		/// Previous restriction type.
		MosaicRestrictionType PreviousRestrictionType;

		/// New restriction type.
		MosaicRestrictionType NewRestrictionType;

	public:
		/// Calculates the real size of a mosaic global restriction \a transaction.
		static constexpr uint64_t CalculateRealSize(const TransactionType&) noexcept {
			return sizeof(TransactionType);
		}
	};

	DEFINE_EMBEDDABLE_TRANSACTION(MosaicGlobalRestriction)

#pragma pack(pop)
}}

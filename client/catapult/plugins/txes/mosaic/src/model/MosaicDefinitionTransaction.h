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
#include "MosaicConstants.h"
#include "MosaicEntityType.h"
#include "MosaicProperties.h"
#include "catapult/model/Transaction.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a mosaic definition transaction body.
	template<typename THeader>
	struct MosaicDefinitionTransactionBody : public THeader {
	private:
		using TransactionType = MosaicDefinitionTransactionBody<THeader>;

	public:
		DEFINE_TRANSACTION_CONSTANTS(Entity_Type_Mosaic_Definition, 1)

	public:
		/// Mosaic identifier.
		/// \note This must match the generated id.
		MosaicId Id;

		/// Mosaic duration
		BlockDuration Duration;

		/// Mosaic nonce.
		MosaicNonce Nonce;

		/// Mosaic flags.
		MosaicFlags Flags;

		/// Mosaic divisibility.
		uint8_t Divisibility;

	public:
		/// Calculates the real size of mosaic definition \a transaction.
		static constexpr uint64_t CalculateRealSize(const TransactionType&) noexcept {
			return sizeof(TransactionType);
		}
	};

	DEFINE_EMBEDDABLE_TRANSACTION(MosaicDefinition)

#pragma pack(pop)
}}

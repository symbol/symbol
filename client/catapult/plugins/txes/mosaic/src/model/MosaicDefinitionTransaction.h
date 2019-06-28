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
		/// Mosaic nonce.
		catapult::MosaicNonce MosaicNonce;

		/// Id of the mosaic.
		/// \note This must match the generated id.
		catapult::MosaicId MosaicId;

		/// Properties header.
		MosaicPropertiesHeader PropertiesHeader;

		// followed by optional properties
		DEFINE_TRANSACTION_VARIABLE_DATA_ACCESSORS(Properties, MosaicProperty)

	private:
		template<typename T>
		static auto* PropertiesPtrT(T& transaction) {
			return transaction.PropertiesHeader.Count ? THeader::PayloadStart(transaction) : nullptr;
		}

	public:
		/// Calculates the real size of mosaic definition \a transaction.
		static constexpr uint64_t CalculateRealSize(const TransactionType& transaction) noexcept {
			return sizeof(TransactionType) + transaction.PropertiesHeader.Count * sizeof(MosaicProperty);
		}
	};

	DEFINE_EMBEDDABLE_TRANSACTION(MosaicDefinition)

#pragma pack(pop)
}}

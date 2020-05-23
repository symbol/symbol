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
#include "MetadataEntityType.h"
#include "MetadataSharedTransaction.h"
#include "catapult/model/ContainerTypes.h"
#include "catapult/utils/ArraySet.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Metadata transaction header with mosaic id target.
	template<typename THeader>
	struct MosaicMetadataTransactionHeader : public MetadataTransactionHeader<THeader> {
		/// Target mosaic identifier.
		UnresolvedMosaicId TargetMosaicId;
	};

	/// Binary layout for a mosaic metadata transaction body.
	template<typename THeader>
	struct MosaicMetadataTransactionBody
			: public BasicMetadataTransactionBody<MosaicMetadataTransactionHeader<THeader>, Entity_Type_Mosaic_Metadata>
	{};

	DEFINE_EMBEDDABLE_TRANSACTION(MosaicMetadata)

#pragma pack(pop)

	/// Extracts addresses of additional accounts that must approve \a transaction.
	inline UnresolvedAddressSet ExtractAdditionalRequiredCosignatories(const EmbeddedMosaicMetadataTransaction& transaction) {
		return { transaction.TargetAddress };
	}
}}


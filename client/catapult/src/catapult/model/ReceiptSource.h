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
#include <stdint.h>

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Receipt source (unique within a block).
	struct ReceiptSource {
	public:
		/// Creates a default receipt source.
		ReceiptSource() : ReceiptSource(0, 0)
		{}

		/// Creates a receipt source around \a primaryId and \a secondaryId.
		ReceiptSource(uint32_t primaryId, uint32_t secondaryId)
				: PrimaryId(primaryId)
				, SecondaryId(secondaryId)
		{}

	public:
		/// Transaction primary source (e.g. index within block).
		uint32_t PrimaryId;

		/// Transaction secondary source (e.g. index within aggregate).
		uint32_t SecondaryId;

	public:
		/// Returns \c true if this receipt source is less than \a rhs.
		constexpr bool operator<(const ReceiptSource& rhs) const {
			return PrimaryId < rhs.PrimaryId || (PrimaryId == rhs.PrimaryId && SecondaryId < rhs.SecondaryId);
		}
	};

#pragma pack(pop)
}}

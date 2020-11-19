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
#include "ReceiptSource.h"
#include "ReceiptType.h"
#include "catapult/types.h"

namespace catapult { namespace model {

	/// Collection of receipts scoped to a unresolved value.
	template<typename TUnresolved, typename TResolved, ReceiptType ResolutionReceiptType>
	class ResolutionStatement {
	public:
#pragma pack(push, 1)

		/// Resolution entry.
		struct ResolutionEntry {
			/// Source of resolution within block.
			ReceiptSource Source;

			/// Resolved value.
			TResolved ResolvedValue;
		};

#pragma pack(pop)

	public:
		/// Creates a statement around \a unresolved value.
		explicit ResolutionStatement(const TUnresolved& unresolved);

	public:
		/// Gets the unresolved value.
		const TUnresolved& unresolved() const;

		/// Gets the number of attached resolution entries.
		size_t size() const;

		/// Gets the resolution entry at \a index.
		const ResolutionEntry& entryAt(size_t index) const;

		/// Calculates a unique hash for this statment.
		Hash256 hash() const;

	public:
		/// Adds a resolution entry for resolving the unresolved value to \a resolved value at \a source.
		void addResolution(const TResolved& resolved, const ReceiptSource& source);

	private:
		// store compiler aligned struct in vector instead of packed struct in order to align all entries in vector on 8-byte boundaries
		struct PaddedResolutionEntry : public ResolutionEntry {
			uint64_t Reserved;
		};

	private:
		TUnresolved m_unresolved;
		std::vector<PaddedResolutionEntry> m_entries;
	};

	/// Address resolution statement.
	using AddressResolutionStatement = ResolutionStatement<UnresolvedAddress, Address, Receipt_Type_Address_Alias_Resolution>;
	extern template class ResolutionStatement<UnresolvedAddress, Address, Receipt_Type_Address_Alias_Resolution>;

	/// Mosaic resolution statement.
	using MosaicResolutionStatement = ResolutionStatement<UnresolvedMosaicId, MosaicId, Receipt_Type_Mosaic_Alias_Resolution>;
	extern template class ResolutionStatement<UnresolvedMosaicId, MosaicId, Receipt_Type_Mosaic_Alias_Resolution>;
}}

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

#include "ResolutionStatement.h"
#include "catapult/crypto/Hashes.h"

namespace catapult { namespace model {

#define RESOLUTION_STATEMENT_T ResolutionStatement<TUnresolved, TResolved, ResolutionReceiptType>

	template<typename TUnresolved, typename TResolved, ReceiptType ResolutionReceiptType>
	RESOLUTION_STATEMENT_T::ResolutionStatement(const TUnresolved& unresolved) : m_unresolved(unresolved)
	{}

	template<typename TUnresolved, typename TResolved, ReceiptType ResolutionReceiptType>
	const TUnresolved& RESOLUTION_STATEMENT_T::unresolved() const {
		return m_unresolved;
	}

	template<typename TUnresolved, typename TResolved, ReceiptType ResolutionReceiptType>
	size_t RESOLUTION_STATEMENT_T::size() const {
		return m_entries.size();
	}

	template<typename TUnresolved, typename TResolved, ReceiptType ResolutionReceiptType>
	const typename RESOLUTION_STATEMENT_T::ResolutionEntry& RESOLUTION_STATEMENT_T::entryAt(size_t index) const {
		return m_entries[index];
	}

	template<typename TUnresolved, typename TResolved, ReceiptType ResolutionReceiptType>
	Hash256 RESOLUTION_STATEMENT_T::hash() const {
		// prepend receipt header to statement
		auto version = static_cast<uint16_t>(1);
		auto type = ResolutionReceiptType;

		crypto::Sha3_256_Builder hashBuilder;
		hashBuilder.update({ reinterpret_cast<const uint8_t*>(&version), sizeof(uint16_t) });
		hashBuilder.update({ reinterpret_cast<const uint8_t*>(&type), sizeof(ReceiptType) });
		hashBuilder.update({ reinterpret_cast<const uint8_t*>(&m_unresolved), sizeof(TUnresolved) });

		for (const auto& entry : m_entries)
			hashBuilder.update({ reinterpret_cast<const uint8_t*>(&entry), sizeof(ResolutionEntry) });

		Hash256 hash;
		hashBuilder.final(hash);
		return hash;
	}

	template<typename TUnresolved, typename TResolved, ReceiptType ResolutionReceiptType>
	void RESOLUTION_STATEMENT_T::addResolution(const TResolved& resolved, const ReceiptSource& source) {
		if (!m_entries.empty()) {
			const auto& lastSource = m_entries.back().Source;
			if (source < lastSource) {
				std::ostringstream out;
				out
						<< "detected out of order resolution - "
						<< "last (" << lastSource.PrimaryId << ", " << lastSource.SecondaryId << ") "
						<< "next (" << source.PrimaryId << ", " << source.SecondaryId << ")";
				CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
			}

			// collapse consecutive resolutions of same value
			if (resolved == m_entries.back().ResolvedValue)
				return;
		}

		PaddedResolutionEntry entry;
		entry.Source = source;
		entry.ResolvedValue = resolved;
		m_entries.push_back(entry);
	}

#undef RESOLUTION_STATEMENT_T

	template class ResolutionStatement<UnresolvedAddress, Address, Receipt_Type_Address_Alias_Resolution>;
	template class ResolutionStatement<UnresolvedMosaicId, MosaicId, Receipt_Type_Mosaic_Alias_Resolution>;
}}

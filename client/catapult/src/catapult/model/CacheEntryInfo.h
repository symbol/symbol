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
#include "TrailingVariableDataLayout.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Cache entry info.
	template<typename TIdentifier>
	struct CacheEntryInfo : public TrailingVariableDataLayout<CacheEntryInfo<TIdentifier>, uint8_t> {
	private:
		using BaseType = TrailingVariableDataLayout<CacheEntryInfo<TIdentifier>, uint8_t>;
		using BaseType::PayloadStart;
		using BaseType::ToTypedPointer;

	public:
		//// Max data size for serialized cache entries.
		static constexpr uint32_t Max_Data_Size = 0x00FFFFFF;

	public:
		/// Size of the entry data.
		uint32_t DataSize;

		/// Cache entry's id.
		TIdentifier Id;

		// followed by data if DataSize > 0
		DEFINE_TRAILING_VARIABLE_DATA_LAYOUT_ACCESSORS(Data, Size)

	public:
		/// Returns \c true if data is available.
		bool HasData() const {
			return DataSize > 0;
		}

		/// Returns \c true if the serialized cache entry is too large.
		bool IsTooLarge() const {
			return DataSize >= Max_Data_Size;
		}

	public:
		/// Calculates the real size of \a cacheEntryInfo.
		static constexpr uint64_t CalculateRealSize(const CacheEntryInfo& cacheEntryInfo) noexcept {
			return sizeof(CacheEntryInfo<TIdentifier>) + cacheEntryInfo.DataSize;
		}
	};

#pragma pack(pop)
}}

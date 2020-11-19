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
#include "RestrictionValueMap.h"
#include "catapult/types.h"

namespace catapult { namespace state {

	/// Mosaic restrictions scoped to an address.
	class MosaicAddressRestriction {
	public:
		/// Sentinel value that triggers restriction removal.
		static constexpr uint64_t Sentinel_Removal_Value = std::numeric_limits<uint64_t>::max();

	public:
		/// Creates a restriction around \a mosaicId and \a address.
		MosaicAddressRestriction(MosaicId mosaicId, Address address);

	public:
		/// Gets the mosaic id.
		MosaicId mosaicId() const;

		/// Gets the address.
		Address address() const;

		/// Gets the number of restriction rules.
		size_t size() const;

		/// Gets all restriction keys.
		std::set<uint64_t> keys() const;

	public:
		/// Gets the value associated with \a key.
		uint64_t get(uint64_t key) const;

		/// Sets the \a value associated with \a key.
		void set(uint64_t key, uint64_t value);

	public:
		MosaicId m_mosaicId;
		Address m_address;
		RestrictionValueMap<uint64_t> m_keyValuePairs;
	};
}}

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

#include "MosaicAddressRestriction.h"

namespace catapult { namespace state {

	MosaicAddressRestriction::MosaicAddressRestriction(MosaicId mosaicId, Address address)
			: m_mosaicId(mosaicId)
			, m_address(address)
	{}

	MosaicId MosaicAddressRestriction::mosaicId() const {
		return m_mosaicId;
	}

	Address MosaicAddressRestriction::address() const {
		return m_address;
	}

	size_t MosaicAddressRestriction::size() const {
		return m_keyValuePairs.size();
	}

	std::set<uint64_t> MosaicAddressRestriction::keys() const {
		return m_keyValuePairs.keys();
	}

	uint64_t MosaicAddressRestriction::get(uint64_t key) const {
		uint64_t value;
		return m_keyValuePairs.tryGet(key, value) ? value : Sentinel_Removal_Value;
	}

	void MosaicAddressRestriction::set(uint64_t key, uint64_t value) {
		if (Sentinel_Removal_Value == value)
			m_keyValuePairs.remove(key);
		else
			m_keyValuePairs.set(key, value);
	}
}}

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

#include "HeightDependentAddress.h"
#include "catapult/exceptions.h"

namespace catapult { namespace model {

	HeightDependentAddress::HeightDependentAddress() : HeightDependentAddress(Address())
	{}

	HeightDependentAddress::HeightDependentAddress(const Address& address) : m_defaultAddress(address)
	{}

	bool HeightDependentAddress::trySet(const Address& address, Height endHeight) {
		if (Height() == endHeight)
			return false;

		if (!m_addresses.empty() && m_addresses.back().second >= endHeight)
			CATAPULT_THROW_INVALID_ARGUMENT("height dependent address overrides must be set with increasing end heights");

		m_addresses.emplace_back(address, endHeight);
		return true;
	}

	Address HeightDependentAddress::get(Height height) const {
		if (height >= Height(1)) {
			for (const auto& pair : m_addresses) {
				if (height < pair.second)
					return pair.first;
			}
		}

		return m_defaultAddress;
	}
}}

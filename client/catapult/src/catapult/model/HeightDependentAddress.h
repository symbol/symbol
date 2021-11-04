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
#include "catapult/types.h"
#include <vector>

namespace catapult { namespace model {

	/// Container that provides a height to address mapping.
	class HeightDependentAddress {
	public:
		/// Creates an empty height dependent address.
		HeightDependentAddress();

		/// Creates a height dependent address around a default \a address.
		explicit HeightDependentAddress(const Address& address);

	public:
		/// Tries to set an address that should be used for all heights between the current highest overloaded height and \a endHeight.
		/// \note Zero \a endHeight is ignored.
		bool trySet(const Address& address, Height endHeight);

		/// Gets the active address at \a height.
		Address get(Height height) const;

	private:
		Address m_defaultAddress;
		std::vector<std::pair<Address, Height>> m_addresses;
	};
}}

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

#include "ResolverContext.h"
#include <cstring>

namespace catapult { namespace model {

	ResolverContext::ResolverContext()
			: ResolverContext(
					[](auto mosaicId) { return MosaicId(mosaicId.unwrap()); },
					[](const auto& address) { return address.template copyTo<Address>(); })
	{}

	ResolverContext::ResolverContext(const MosaicResolver& mosaicResolver, const AddressResolver& addressResolver)
			: m_mosaicResolver(mosaicResolver)
			, m_addressResolver(addressResolver)
	{}

	MosaicId ResolverContext::resolve(UnresolvedMosaicId mosaicId) const {
		return m_mosaicResolver(mosaicId);
	}

	Address ResolverContext::resolve(const UnresolvedAddress& address) const {
		return m_addressResolver(address);
	}
}}

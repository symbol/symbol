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

#include "ResolverTestUtils.h"

namespace catapult { namespace test {

	model::ResolverContext CreateResolverContextWithCustomDoublingMosaicResolver() {
		// use custom mosaic resolver but default address resolver
		return model::ResolverContext(
				[](const auto& unresolved) { return MosaicId(unresolved.unwrap() * 2); },
				[](const auto& unresolved) { return model::ResolverContext().resolve(unresolved); });
	}

	model::ResolverContext CreateResolverContextXor() {
		return model::ResolverContext(
				[](const auto& unresolved) {
					return MosaicId(unresolved.unwrap() ^ 0xFFFFFFFFFFFFFFFF);
				},
				[](const auto& unresolved) {
					auto i = 0u;
					Address resolved;
					for (const auto& byte : unresolved)
						resolved[i++] = byte ^ 0xFF;

					return resolved;
				});
	}

	UnresolvedMosaicId UnresolveXor(MosaicId mosaicId) {
		return UnresolvedMosaicId(mosaicId.unwrap() ^ 0xFFFFFFFFFFFFFFFF);
	}

	UnresolvedAddress UnresolveXor(const Address& address) {
		auto i = 0u;
		UnresolvedAddress unresolved;
		for (auto byte : address)
			unresolved[i++] = byte ^ 0xFF;

		return unresolved;
	}
}}

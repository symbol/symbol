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

#include "AddressTestUtils.h"
#include "sdk/src/extensions/ConversionExtensions.h"
#include "catapult/model/Address.h"
#include "tests/test/nodeps/Random.h"

namespace catapult { namespace test {

	Address GenerateRandomAddress() {
		return GenerateRandomAddress(model::NetworkIdentifier::Private_Test);
	}

	Address GenerateRandomAddress(model::NetworkIdentifier networkIdentifier) {
		auto publicKey = GenerateRandomByteArray<Key>();
		return model::PublicKeyToAddress(publicKey, networkIdentifier);
	}

	UnresolvedAddress GenerateRandomUnresolvedAddress() {
		return extensions::CopyToUnresolvedAddress(GenerateRandomAddress());
	}

	std::vector<Address> GenerateRandomAddresses(size_t count) {
		std::vector<Address> addresses;
		for (auto i = 0u; i < count; ++i)
			addresses.push_back(test::GenerateRandomAddress());

		return addresses;
	}

	std::shared_ptr<model::UnresolvedAddressSet> GenerateRandomUnresolvedAddressSetPointer(size_t count) {
		auto pAddresses = std::make_shared<model::UnresolvedAddressSet>();
		for (auto i = 0u; i < count; ++i)
			pAddresses->emplace(GenerateRandomUnresolvedAddress());

		return pAddresses;
	}
}}

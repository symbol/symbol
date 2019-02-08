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

#include "CryptoUtils.h"
#include "Hashes.h"
#include "PrivateKey.h"
#include "SecureZero.h"
#ifdef SIGNATURE_SCHEME_NIS1
#include <algorithm>
#endif

namespace catapult { namespace crypto {

	void HashPrivateKey(const PrivateKey& privateKey, Hash512& hash) {
#ifdef SIGNATURE_SCHEME_NIS1
		Key reversedKey;
		std::reverse_copy(privateKey.begin(), privateKey.end(), reversedKey.begin());
		Keccak_512(reversedKey, hash);
		SecureZero(reversedKey.data(), Key_Size);
#else
		Sha3_512({ privateKey.data(), privateKey.size() }, hash);
#endif
	}
}}

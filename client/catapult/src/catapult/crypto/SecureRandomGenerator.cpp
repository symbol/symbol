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

#include "SecureRandomGenerator.h"
#include "catapult/exceptions.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <openssl/rand.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

namespace catapult { namespace crypto {

	SecureRandomGenerator::result_type SecureRandomGenerator::operator()() {
		SecureRandomGenerator::result_type result = 0;
		fill(reinterpret_cast<uint8_t*>(&result), sizeof(SecureRandomGenerator::result_type));
		return result;
	}

	void SecureRandomGenerator::fill(uint8_t* pOut, size_t count) {
		if (!RAND_bytes(pOut, static_cast<int>(count)))
			CATAPULT_THROW_RUNTIME_ERROR("unable to generate secure random numbers");
	}
}}

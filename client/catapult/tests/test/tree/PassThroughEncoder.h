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
#include "catapult/crypto/Hashes.h"
#include "catapult/types.h"
#include <string>

namespace catapult {
namespace test {

    /// Patricia tree pass-through encoder.
    class PassThroughEncoder {
    public:
        using KeyType = uint32_t;
        using ValueType = std::string;

    public:
        /// Encodes \a key.
        static const KeyType& EncodeKey(const KeyType& key)
        {
            return key;
        }

        /// Encodes \a value.
        static Hash256 EncodeValue(const ValueType& value)
        {
            Hash256 valueHash;
            crypto::Sha3_256(StringToBuffer(value), valueHash);
            return valueHash;
        }

    private:
        static RawBuffer StringToBuffer(const std::string& str)
        {
            return { reinterpret_cast<const uint8_t*>(str.data()), str.size() };
        }
    };
}
}

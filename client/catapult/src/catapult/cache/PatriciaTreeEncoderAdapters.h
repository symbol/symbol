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
#include "catapult/cache_db/KeySerializers.h"
#include "catapult/crypto/Hashes.h"

namespace catapult { namespace cache {

	/// Encoder adapter that hashes values but not keys.
	template<typename TSerializer>
	class SerializerPlainKeyEncoder {
	public:
		using KeyType = typename TSerializer::KeyType;
		using ValueType = typename TSerializer::ValueType;

	public:
		/// Encodes \a key by returning it without modification.
		static constexpr const KeyType& EncodeKey(const KeyType& key) {
			return key;
		}

		/// Encodes \a value by hashing it.
		static Hash256 EncodeValue(const ValueType& value) {
			auto encodedData = TSerializer::SerializeValue(value);

			Hash256 valueHash;
			crypto::Sha3_256({ reinterpret_cast<const uint8_t*>(encodedData.data()), encodedData.size() }, valueHash);
			return valueHash;
		}
	};

	/// Encoder adapter that hashes values and keys.
	template<typename TSerializer>
	class SerializerHashedKeyEncoder : public SerializerPlainKeyEncoder<TSerializer> {
	public:
		/// Encodes \a key by hashing it.
		static Hash256 EncodeKey(const typename SerializerPlainKeyEncoder<TSerializer>::KeyType& key) {
			Hash256 keyHash;
			crypto::Sha3_256(SerializeKey(key), keyHash);
			return keyHash;
		}
	};
}}

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
#include <string>
#include <vector>

namespace catapult { namespace utils {

	const uint32_t Base32_Decoded_Block_Size = 5;
	const uint32_t Base32_Encoded_Block_Size = 8;

	// region encode

	/// Gets the encoded size of decoded data with size \a decodedSize.
	constexpr size_t GetEncodedDataSize(size_t decodedSize) {
		return decodedSize / Base32_Decoded_Block_Size * Base32_Encoded_Block_Size;
	}

	/// Tries to encode an array of bytes pointed to by \a data into \a encodedData.
	/// \note The size must be a multiple of 5.
	bool TryBase32Encode(const RawBuffer& data, const MutableRawString& encodedData);

	/// Encodes an array of bytes pointed to by \a data into \a encodedData.
	/// \note The size must be a multiple of 5.
	void Base32Encode(const RawBuffer& data, const MutableRawString& encodedData);

	/// Encodes an array of bytes pointed to by \a data. The size must be a multiple of 5.
	std::string Base32Encode(const RawBuffer& data);

	// endregion

	// region decode

	/// Gets the decoded size of encoded data with size \a encodedSize.
	constexpr size_t GetDecodedDataSize(size_t encodedSize) {
		return encodedSize / Base32_Encoded_Block_Size * Base32_Decoded_Block_Size;
	}

	/// Tries to decode a base32 encoded string pointed to by \a encodedData into \a data.
	/// \note The string length must be a multiple of 8.
	bool TryBase32Decode(const RawString& encodedData, const MutableRawBuffer& data);

	/// Decodes a base32 encoded string pointed to by \a encodedData into \a data.
	/// \note The string length must be a multiple of 8.
	void Base32Decode(const RawString& encodedData, const MutableRawBuffer& data);

	/// Decodes a base32 encoded string pointed to by \a encodedData. The string length must be a multiple of 8.
	template<size_t N>
	std::array<uint8_t, N> Base32Decode(const RawString& encodedData) {
		std::array<uint8_t, N> data;
		Base32Decode(encodedData, data);
		return data;
	}

	// endregion
}}

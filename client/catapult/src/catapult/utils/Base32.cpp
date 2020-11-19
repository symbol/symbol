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

#include "Base32.h"
#include "catapult/exceptions.h"
#include <array>
#include <sstream>

namespace catapult { namespace utils {

	namespace {
		const char* Allowed_Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
		constexpr uint32_t Decoded_Block_Size = Base32_Decoded_Block_Size;
		constexpr uint32_t Encoded_Block_Size = Base32_Encoded_Block_Size;

		bool TryDecodeChar(const char c, uint8_t& b) {
			if (c >= 'A' && c <= 'Z')
				b = static_cast<uint8_t>(c - 'A');
			else if (c >= '2' && c <= '7')
				b = static_cast<uint8_t>(c - '2' + 26);
			else
				return false;

			return true;
		}

		void EncodeBlock(const uint8_t* pData, char* it) {
			*it++ = Allowed_Chars[pData[0] >> 3];
			*it++ = Allowed_Chars[(pData[0] & 0x07) << 2 | pData[1] >> 6];
			*it++ = Allowed_Chars[(pData[1] & 0x3E) >> 1];
			*it++ = Allowed_Chars[(pData[1] & 0x01) << 4 | pData[2] >> 4];
			*it++ = Allowed_Chars[(pData[2] & 0x0F) << 1 | pData[3] >> 7];
			*it++ = Allowed_Chars[(pData[3] & 0x7F) >> 2];
			*it++ = Allowed_Chars[(pData[3] & 0x03) << 3 | pData[4] >> 5];
			*it = Allowed_Chars[pData[4] & 0x1F];
		}

		bool TryDecodeBlock(const char* encodedData, uint8_t* it) {
			std::array<uint8_t, Encoded_Block_Size> bytes;
			for (auto i = 0u; i < Encoded_Block_Size; i++) {
				if (!TryDecodeChar(encodedData[i], bytes[i]))
					return false;
			}

			*it++ = static_cast<uint8_t>(bytes[0] << 3 | bytes[1] >> 2);
			*it++ = static_cast<uint8_t>((bytes[1] & 0x03) << 6 | bytes[2] << 1 | bytes[3] >> 4);
			*it++ = static_cast<uint8_t>((bytes[3] & 0x0F) << 4 | bytes[4] >> 1);
			*it++ = static_cast<uint8_t>((bytes[4] & 0x01) << 7 | bytes[5] << 2 | bytes[6] >> 3);
			*it = static_cast<uint8_t>((bytes[6] & 0x07) << 5 | bytes[7]);
			return true;
		}

		enum class EncodeResult {
			Success,
			OutputBufferTooSmall,
			InvalidSize
		};

		EncodeResult TryBase32EncodeInternal(const RawBuffer& data, const MutableRawString& encodedData) {
			if (encodedData.Size < GetEncodedDataSize(data.Size))
				return EncodeResult::OutputBufferTooSmall;

			if (0 != data.Size % Decoded_Block_Size)
				return EncodeResult::InvalidSize;

			for (auto i = 0u; i < data.Size / Decoded_Block_Size; ++i)
				EncodeBlock(&data.pData[i * Decoded_Block_Size], &encodedData.pData[i * Encoded_Block_Size]);

			return EncodeResult::Success;
		}

		enum class DecodeResult {
			Success,
			IllegalCharacter,
			OutputBufferTooSmall,
			InvalidSize
		};

		DecodeResult TryBase32DecodeInternal(const RawString& encodedData, const MutableRawBuffer& data) {
			if (data.Size < GetDecodedDataSize(encodedData.Size))
				return DecodeResult::OutputBufferTooSmall;

			if (0 != encodedData.Size % Encoded_Block_Size)
				return DecodeResult::InvalidSize;

			for (auto i = 0u; i < encodedData.Size / Encoded_Block_Size; ++i) {
				if (!TryDecodeBlock(&encodedData.pData[i * Encoded_Block_Size], data.pData + i * Decoded_Block_Size))
					return DecodeResult::IllegalCharacter;
			}

			return DecodeResult::Success;
		}
	}

	bool TryBase32Encode(const RawBuffer& data, const MutableRawString& encodedData) {
		return EncodeResult::Success == TryBase32EncodeInternal(data, encodedData);
	}

	void Base32Encode(const RawBuffer& data, const MutableRawString& encodedData) {
		auto result = TryBase32EncodeInternal(data, encodedData);
		switch (result) {
		case EncodeResult::Success:
			break;

		case EncodeResult::OutputBufferTooSmall:
			CATAPULT_THROW_RUNTIME_ERROR("base32 output buffer is too small");

		case EncodeResult::InvalidSize:
			CATAPULT_THROW_RUNTIME_ERROR_1("byte array size is not multiple", Decoded_Block_Size);
		}
	}

	std::string Base32Encode(const RawBuffer& data) {
		std::string encodedString;
		encodedString.resize(GetEncodedDataSize(data.Size));
		Base32Encode(data, encodedString);
		return encodedString;
	}

	bool TryBase32Decode(const RawString& encodedData, const MutableRawBuffer& data) {
		return DecodeResult::Success == TryBase32DecodeInternal(encodedData, data);
	}

	void Base32Decode(const RawString& encodedData, const MutableRawBuffer& data) {
		auto result = TryBase32DecodeInternal(encodedData, data);
		switch (result) {
		case DecodeResult::Success:
			break;

		case DecodeResult::IllegalCharacter:
			CATAPULT_THROW_RUNTIME_ERROR("illegal character in base32 string");

		case DecodeResult::OutputBufferTooSmall:
			CATAPULT_THROW_RUNTIME_ERROR("base32 output buffer is too small");

		case DecodeResult::InvalidSize:
			CATAPULT_THROW_RUNTIME_ERROR_1("string length is not multiple", Encoded_Block_Size);
		}
	}
}}

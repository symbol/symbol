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

#include "PrivateKey.h"
#include "SecureZero.h"
#include "catapult/utils/HexParser.h"
#include <algorithm>

namespace catapult { namespace crypto {

	namespace {
		class SecureZeroGuard {
		public:
			SecureZeroGuard(Key& key) : SecureZeroGuard(key.data(), key.size())
			{}

			SecureZeroGuard(uint8_t* pData, size_t dataSize) : m_pData(pData), m_dataSize(dataSize)
			{}

			~SecureZeroGuard() {
				SecureZero(m_pData, m_dataSize);
			}

		private:
			uint8_t* m_pData;
			size_t m_dataSize;
		};
	}

	PrivateKey::PrivateKey(PrivateKey&& rhs) {
		SecureZeroGuard guard(rhs.m_key);
		m_key = std::move(rhs.m_key);
	}

	PrivateKey& PrivateKey::operator=(PrivateKey&& rhs) {
		SecureZeroGuard guard(rhs.m_key);
		m_key = std::move(rhs.m_key);
		return *this;
	}

	PrivateKey::~PrivateKey() {
		SecureZero(m_key);
	}

	PrivateKey PrivateKey::FromString(const char* const pRawKey, size_t keySize) {
		PrivateKey key;
		utils::ParseHexStringIntoContainer(pRawKey, keySize, key.m_key);
		return key;
	}

	PrivateKey PrivateKey::FromString(const std::string& str) {
		return FromString(str.c_str(), str.size());
	}

	PrivateKey PrivateKey::FromStringSecure(char* pRawKey, size_t keySize) {
		SecureZeroGuard guard(reinterpret_cast<uint8_t*>(pRawKey), keySize);
		return FromString(pRawKey, keySize);
	}

	PrivateKey PrivateKey::Generate(const supplier<uint8_t>& generator) {
		PrivateKey key;
		std::generate_n(key.m_key.begin(), key.m_key.size(), generator);
		return key;
	}

	bool PrivateKey::operator==(const PrivateKey& rhs) const {
		return m_key == rhs.m_key;
	}

	bool PrivateKey::operator!=(const PrivateKey& rhs) const {
		return !(*this == rhs);
	}
}}

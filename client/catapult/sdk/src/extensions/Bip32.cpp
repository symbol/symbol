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

#include "Bip32.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/KeyPair.h"

namespace catapult { namespace extensions {

	namespace {
		Hash512 Hmac_Sha512(const RawBuffer& key, const RawBuffer& data) {
			Hash512 hmacResult;
			crypto::Hmac_Sha512(key, data, hmacResult);
			return hmacResult;
		}
	}

	Bip32Node::Bip32Node(const RawBuffer& key, const RawBuffer& data) : Bip32Node(Hmac_Sha512(key, data))
	{}

	Bip32Node::Bip32Node(Hash512&& hmacResult)
			: m_keyPair(crypto::KeyPair::FromPrivate(crypto::PrivateKey::FromBufferSecure({ &hmacResult[0], crypto::PrivateKey::Size }))) {
		std::memcpy(&m_chainCode[0], &hmacResult[crypto::PrivateKey::Size], Hash512::Size - crypto::PrivateKey::Size);
	}

	const Hash256& Bip32Node::chainCode() const {
		return m_chainCode;
	}

	const Key& Bip32Node::publicKey() const {
		return m_keyPair.publicKey();
	}

#ifdef _MSC_VER
#define BSWAP(VAL) _byteswap_ulong(VAL)
#else
#define BSWAP(VAL) __builtin_bswap32(VAL)
#endif

	Bip32Node Bip32Node::derive(uint32_t id) {
		static constexpr size_t Hmac_Data_Size = 1 + crypto::PrivateKey::Size + sizeof(uint32_t);
		static constexpr size_t Id_Offset = 1 + crypto::PrivateKey::Size;

		std::array<uint8_t, Hmac_Data_Size> hmacData;
		hmacData[0] = 0;
		std::memcpy(&hmacData[1], m_keyPair.privateKey().data(), m_keyPair.privateKey().size());

		// write id as big endian and set high bit
		auto reversedId = BSWAP(id);
		std::memcpy(&hmacData[Id_Offset], &reversedId, sizeof(uint32_t));
		hmacData[Id_Offset] |= 0x80;
		return Bip32Node(m_chainCode, hmacData);
	}

	Bip32Node Bip32Node::derive(const std::vector<uint32_t>& path) {
		auto iter = path.begin();
		auto nextNode = derive(*iter++);
		for (; path.end() != iter ; ++iter)
			nextNode = nextNode.derive(*iter);

		return nextNode;
	}

	Bip32Node Bip32Node::FromSeed(const RawBuffer& seed) {
		static constexpr const char* Root_Node_Prefix = "ed25519 seed";
		return Bip32Node({ reinterpret_cast<const uint8_t*>(Root_Node_Prefix), strlen(Root_Node_Prefix) }, seed);
	}

	crypto::KeyPair Bip32Node::ExtractKeyPair(Bip32Node&& node) {
		return std::move(node.m_keyPair);
	}
}}

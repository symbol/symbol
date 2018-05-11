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

#pragma once
#include "KeyGenerator.h"
#include "PrivateKey.h"
#include "catapult/types.h"

namespace catapult { namespace crypto {

#ifdef SPAMMER_TOOL
#pragma pack(push, 16)
#endif

	/// Represents a pair of private key with associated public key.
	class KeyPair final {
	private:
		explicit KeyPair(PrivateKey&& privateKey) : m_privateKey(std::move(privateKey)) {
			ExtractPublicKeyFromPrivateKey(m_privateKey, m_publicKey);
		}

	public:
		/// Creates a key pair from \a privateKey.
		static auto FromPrivate(PrivateKey&& privateKey) {
			return KeyPair(std::move(privateKey));
		}

		/// Creates a key pair from \a privateKey.
		static auto FromString(const std::string& privateKey) {
			return FromPrivate(PrivateKey::FromString(privateKey));
		}

		/// Returns a private key of a key pair.
		const auto& privateKey() const {
			return m_privateKey;
		}

		/// Returns a public key of a key pair.
		const auto& publicKey() const {
			return m_publicKey;
		}

	private:
		PrivateKey m_privateKey;
		Key m_publicKey;
	};

#ifdef SPAMMER_TOOL
#pragma pack(pop)
#endif
}}

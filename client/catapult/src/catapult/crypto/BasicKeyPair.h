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
#include <string>

namespace catapult { namespace crypto {

#ifdef SPAMMER_TOOL
#pragma pack(push, 16)
#endif

	/// Represents a pair of private key with associated public key.
	template<typename TTraits>
	class BasicKeyPair final {
	public:
		using PublicKey = typename TTraits::PublicKey;
		using PrivateKey = typename TTraits::PrivateKey;

	private:
		explicit BasicKeyPair(PrivateKey&& privateKey) : m_privateKey(std::move(privateKey)) {
			TTraits::ExtractPublicKeyFromPrivateKey(m_privateKey, m_publicKey);
		}

	public:
		/// Creates a key pair from \a privateKey.
		static auto FromPrivate(PrivateKey&& privateKey) {
			return BasicKeyPair(std::move(privateKey));
		}

		/// Creates a key pair from \a privateKey string.
		static auto FromString(const std::string& privateKey) {
			return FromPrivate(PrivateKey::FromString(privateKey));
		}

	public:
		/// Gets the private key of the key pair.
		const auto& privateKey() const {
			return m_privateKey;
		}

		/// Gets the public key of the key pair.
		const auto& publicKey() const {
			return m_publicKey;
		}

	private:
		PrivateKey m_privateKey;
		PublicKey m_publicKey;
	};

#ifdef SPAMMER_TOOL
#pragma pack(pop)
#endif
}}

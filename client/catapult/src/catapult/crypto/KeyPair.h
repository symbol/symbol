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
#pragma pack(push, 16)
#endif
}}

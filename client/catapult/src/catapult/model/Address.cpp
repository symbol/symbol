#include "Address.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/utils/Base32.h"
#include "catapult/utils/Casting.h"
#include "catapult/exceptions.h"

namespace catapult { namespace model {
	const uint8_t Checksum_Size = 4;

	Address StringToAddress(const std::string& str) {
		if (Address_Encoded_Size != str.size())
			CATAPULT_THROW_RUNTIME_ERROR_1("encoded address has wrong size", str.size());

		return utils::Base32Decode<Address_Decoded_Size>(str);
	}

	std::string AddressToString(const Address& address) {
		return utils::Base32Encode(address);
	}

	Address PublicKeyToAddress(const Key& publicKey, NetworkIdentifier networkIdentifier) {
		// step 1: sha3 hash of the public key
		Hash256 publicKeyHash;
		crypto::Sha3_256(publicKey, publicKeyHash);

		// step 2: ripemd160 hash of (1)
		Address decoded;
		crypto::Ripemd160(publicKeyHash, reinterpret_cast<Hash160&>(decoded[1]));

		// step 3: add network identifier byte in front of (2)
		decoded[0] = utils::to_underlying_type(networkIdentifier);

		// step 4: concatenate (3) and the checksum of (3)
		Hash256 step3Hash;
		crypto::Sha3_256(RawBuffer{ decoded.data(), Hash160_Size + 1 }, step3Hash);
		std::copy(step3Hash.cbegin(), step3Hash.cbegin() + Checksum_Size, decoded.begin() + Hash160_Size + 1);

		return decoded;
	}

	bool IsValidAddress(const Address& address, NetworkIdentifier networkIdentifier) {
		if (utils::to_underlying_type(networkIdentifier) != address[0])
			return false;

		Hash256 hash;
		auto checksumBegin = Address_Decoded_Size - Checksum_Size;
		crypto::Sha3_256(RawBuffer{ address.data(), checksumBegin }, hash);

		return std::equal(hash.begin(), hash.begin() + Checksum_Size, address.begin() + checksumBegin);
	}

	bool IsValidEncodedAddress(const std::string& encoded, NetworkIdentifier networkIdentifier) {
		if (Address_Encoded_Size != encoded.size())
			return false;

		Address decoded;
		return utils::TryBase32Decode(encoded, decoded) && IsValidAddress(decoded, networkIdentifier);
	}
}}

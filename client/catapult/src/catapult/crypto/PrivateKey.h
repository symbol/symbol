#pragma once
#include "catapult/types.h"
#include <functional>

namespace catapult { namespace crypto {

#ifdef SPAMMER_TOOL
#pragma pack(push, 16)
#endif

	/// Represents a private key.
	class PrivateKey final {
	public:
		/// Creates a private key.
		PrivateKey() = default;

		/// Destroys the private key.
		~PrivateKey();

	public:
		/// The move constructor.
		PrivateKey(PrivateKey&& rhs);

		/// The move assignment operator.
		PrivateKey& operator=(PrivateKey&& rhs);

	public:
		/// Creates a private key from \a str.
		static PrivateKey FromString(const std::string& str);

		/// Creates a private key from \a pRawKey with size \a keySize and securely erases \a pRawKey.
		static PrivateKey FromStringSecure(char* pRawKey, size_t keySize);

		/// Generates a new private key using the specified byte \a generator.
		static PrivateKey Generate(const std::function<uint8_t()>& generator);

	public:
		/// Returns a const iterator to the beginning of the raw key.
		inline auto cbegin() const { return m_key.cbegin(); }

		/// Returns a const iterator to the end of the raw key.
		inline auto cend() const { return m_key.cend(); }

		/// Returns the size of the key.
		inline auto size() const { return m_key.size(); }

		/// Returns a const pointer to the raw key.
		inline auto data() const { return m_key.data(); }

	public:
		/// Returns \c true if this key and \a rhs are equal.
		bool operator==(const PrivateKey& rhs) const;

		/// Returns \c true if this key and \a rhs are not equal.
		bool operator!=(const PrivateKey& rhs) const;

	private:
		static PrivateKey FromString(const char* const pRawKey, size_t keySize);

	private:
		Key m_key;
	};

#ifdef SPAMMER_TOOL
#pragma pack(pop)
#endif
}}

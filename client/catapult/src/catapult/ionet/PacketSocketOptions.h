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
#include "IpProtocol.h"
#include "catapult/utils/TimeSpan.h"
#include "catapult/functions.h"
#include <filesystem>

namespace boost {
	namespace asio {
		namespace ssl {
			class context;
			class verify_context;
		}
	}
}

namespace catapult { namespace ionet {

	/// Context passed to ssl verify context predicate.
	class PacketSocketSslVerifyContext {
	public:
		/// Creates a default context.
		PacketSocketSslVerifyContext();

		/// Creates a context around \a preverified, \a verifyContext and \a publicKey.
		PacketSocketSslVerifyContext(bool preverified, boost::asio::ssl::verify_context& verifyContext, Key& publicKey);

	public:
		/// Gets the preverified status.
		bool preverified() const;

		/// Gets the asio verify context.
		boost::asio::ssl::verify_context& asioVerifyContext();

		/// Gets the public key.
		const Key& publicKey() const;

		/// Sets the public key to \a publicKey.
		void setPublicKey(const Key& publicKey);

	private:
		bool m_preverified;
		boost::asio::ssl::verify_context* m_pVerifyContext;
		Key* m_pPublicKey;
		Key m_publicKeyBacking;
	};

	/// Packet socket ssl options.
	struct PacketSocketSslOptions {
		/// Supplies an ssl context.
		supplier<boost::asio::ssl::context&> ContextSupplier;

		/// Callback used to verify ssl certificates.
		supplier<predicate<PacketSocketSslVerifyContext&>> VerifyCallbackSupplier;
	};

	/// Packet socket options.
	struct PacketSocketOptions {
		/// Handshake timeout when accepting an incoming connection.
		/// \note Timeouts are applied at higher levels when making an outgoing connection.
		utils::TimeSpan AcceptHandshakeTimeout;

		/// Initial working buffer size.
		size_t WorkingBufferSize;

		/// Working buffer sensitivity.
		size_t WorkingBufferSensitivity;

		/// Maximum packet data size.
		size_t MaxPacketDataSize;

		/// Outgoing connection protocols.
		IpProtocol OutgoingProtocols;

		/// Ssl options.
		PacketSocketSslOptions SslOptions;
	};

	/// Creates an ssl context supplier given the specified certificates in \a certificateDirectory.
	supplier<boost::asio::ssl::context&> CreateSslContextSupplier(const std::filesystem::path& certificateDirectory);

	/// Creates an ssl verify callback supplier.
	supplier<predicate<PacketSocketSslVerifyContext&>> CreateSslVerifyCallbackSupplier();
}}

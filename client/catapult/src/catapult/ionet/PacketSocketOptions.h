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
#include "catapult/functions.h"
#include <boost/filesystem/path.hpp>

namespace boost {
	namespace asio {
		namespace ssl {
			class context;
			class verify_context;
		}
	}
}

namespace catapult { namespace ionet {

	/// Packet socket ssl options.
	struct PacketSocketSslOptions {
		/// Supplies an ssl context.
		supplier<boost::asio::ssl::context&> ContextSupplier;

		/// Callback used to verify ssl certificates.
		predicate<bool, boost::asio::ssl::verify_context&> VerifyCallback;
	};

	/// Packet socket options.
	struct PacketSocketOptions {
		/// Initial working buffer size.
		size_t WorkingBufferSize;

		/// Working buffer sensitivity.
		size_t WorkingBufferSensitivity;

		/// Maximum packet data size.
		size_t MaxPacketDataSize;

		/// Ssl options.
		PacketSocketSslOptions SslOptions;
	};

	/// Creates an ssl context supplier given the specified certificate directory (\a certificateDirectory).
	supplier<boost::asio::ssl::context&> CreateSslContextSupplier(const boost::filesystem::path& certificateDirectory);
}}

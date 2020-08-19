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
#include "CertificateDirectoryGenerator.h"
#include "tools/ToolThreadUtils.h"
#include "catapult/api/ChainApi.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/ionet/Node.h"
#include <boost/asio/ssl.hpp>

namespace catapult { namespace tools { namespace ssl {

	/// Ssl client.
	class SslClient {
	public:
		/// Creates ssl client around \a pool, \a caKeyPair, \a certificateDirectory and \a scenarioId.
		SslClient(thread::IoThreadPool& pool, crypto::KeyPair&& caKeyPair, const std::string& certificateDirectory, ScenarioId scenarioId);

	public:
		/// Connects to \a nodeEndpoint and retrieves chain statistics.
		api::ChainStatistics connect(const ionet::NodeEndpoint& nodeEndpoint);

	private:
		thread::IoThreadPool& m_pool;
		std::shared_ptr<boost::asio::ssl::context> m_pSslContext;
	};
}}}

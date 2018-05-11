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
#include "NetworkConnections.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/model/RangeTypes.h"
#include "catapult/state/TimestampedHash.h"

namespace catapult { namespace model { struct DetachedCosignature; } }

namespace catapult { namespace tools {

	/// Class for sending entities to api nodes and reading data from p2p nodes.
	class ApiSender {
	private:
		using Transactions = std::vector<std::shared_ptr<const model::Transaction>>;

	public:
		/// Creates an api sender around \a resourcesPath.
		explicit ApiSender(const std::string& resourcesPath);

	public:
		/// Gets the local node configuration.
		const config::LocalNodeConfiguration& config();

	public:
		/// Sends \a payload and then waits for two blocks. The send result is logged using \a entityName.
		void sendAndWait(const ionet::PacketPayload& payload, const std::string& entityName);

		/// Sends \a transactions in a packet with \a packetType, logs the send result using \a entityName and waits for two blocks.
		void sendTransactionsAndWait(const Transactions& transactions, ionet::PacketType packetType, const std::string& entityName);

		/// Sends \a cosignatures and then waits for two blocks.
		void sendCosignaturesAndWait(const std::vector<model::DetachedCosignature>& cosignatures);

		/// Given a range of \a hashes gets the confirmed timestamped hashes.
		thread::future<state::TimestampedHashRange> confirmTimestampedHashes(state::TimestampedHashRange&& hashes) const;

		/// Given a range of \a addresses gets the corresponding account infos.
		thread::future<model::AccountInfoRange> accountInfos(model::AddressRange&& addresses) const;

		/// Waits for the network to produce \a numBlocks blocks.
		bool waitForBlocks(size_t numBlocks);

	private:
		const config::LocalNodeConfiguration m_config;
		NetworkConnections m_apiConnections;
		NetworkConnections m_p2pConnections;
	};
}}

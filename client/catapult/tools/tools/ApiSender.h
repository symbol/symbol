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

		/// Sends \a transactions and then waits for two blocks. The payload has type \a packetType.
		/// The send result is logged using \a entityName.
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

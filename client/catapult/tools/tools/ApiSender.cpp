#include "ApiSender.h"
#include "ToolConfigurationUtils.h"
#include "ToolNetworkUtils.h"
#include "Waits.h"
#include "catapult/extensions/RemoteDiagnosticApi.h"
#include "catapult/model/Cosignature.h"
#include "catapult/model/Transaction.h"
#include "catapult/thread/Future.h"

namespace catapult { namespace tools {

	ApiSender::ApiSender(const std::string& resourcesPath)
			: m_config(LoadConfiguration(resourcesPath))
			, m_apiConnections(LoadOptionalApiPeers(resourcesPath, m_config.BlockChain.Network.Identifier))
			, m_p2pConnections(LoadPeers(resourcesPath, m_config.BlockChain.Network.Identifier)) {
		m_apiConnections.connectAll().get();
		m_p2pConnections.connectAll().get();
	}

	const config::LocalNodeConfiguration& ApiSender::config() {
		return m_config;
	}

	void ApiSender::sendAndWait(const ionet::PacketPayload& payload, const std::string& entityName) {
		std::atomic_bool finished(false);
		auto pIo = m_apiConnections.pickOne().io();
		pIo->write(payload, [&finished, entityName](auto code) {
			CATAPULT_LOG(info) << entityName << " send result: " << code;
			finished = true;
		});

		while (!finished)
			std::this_thread::yield();

		pIo.reset();

		// wait for two blocks
		WaitForBlocks(m_p2pConnections, 2);
	}

	void ApiSender::sendTransactionsAndWait(
			const Transactions& transactions,
			ionet::PacketType packetType,
			const std::string& entityName) {
		auto payload = ionet::PacketPayload::FromEntities(packetType, transactions);
		sendAndWait(payload, entityName);
	}

	void ApiSender::sendCosignaturesAndWait(const std::vector<model::DetachedCosignature>& cosignatures) {
		auto range = model::EntityRange<model::DetachedCosignature>::CopyFixed(
			reinterpret_cast<const uint8_t*>(cosignatures.data()),
			cosignatures.size());
		auto payload = ionet::PacketPayload::FromFixedSizeRange(ionet::PacketType::Push_Detached_Cosignatures, std::move(range));
		sendAndWait(payload, "cosignatures");
	}

	thread::future<state::TimestampedHashRange> ApiSender::confirmTimestampedHashes(state::TimestampedHashRange&& hashes) const {
		auto pPacketIo = m_p2pConnections.pickOne().io();
		auto pDiagnosticApi = extensions::CreateRemoteDiagnosticApi(*pPacketIo);
		return pDiagnosticApi->confirmTimestampedHashes(std::move(hashes))
			.then([pPacketIo](auto&& future) { return future.get(); });
	}

	thread::future<model::AccountInfoRange> ApiSender::accountInfos(model::AddressRange&& addresses) const {
		auto pPacketIo = m_p2pConnections.pickOne().io();
		auto pDiagnosticApi = extensions::CreateRemoteDiagnosticApi(*pPacketIo);
		return pDiagnosticApi->accountInfos(std::move(addresses))
			.then([pPacketIo](auto&& future) { return future.get(); });
	}

	bool ApiSender::waitForBlocks(size_t numBlocks) {
		return WaitForBlocks(m_p2pConnections, numBlocks);
	}
}}

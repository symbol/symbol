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
#include "catapult/extensions/RemoteDiagnosticApi.h"
#include "catapult/io/BufferInputStreamAdapter.h"
#include "catapult/ionet/PacketPayloadFactory.h"
#include <unordered_map>

namespace catapult { namespace tools {

	/// Extracts ids from \a transactions using \a getter into a range.
	template<typename TIdType, typename TGetter, typename TTransactions>
	auto GetIdRangeFromTransactions(const TTransactions& transactions, const TGetter& getter) {
		uint8_t* pRangeData;
		auto range = model::EntityRange<TIdType>::PrepareFixed(transactions.size(), &pRangeData);
		auto* pData = reinterpret_cast<TIdType*>(pRangeData);

		for (const auto& pTransaction : transactions)
			*pData++ = getter(*pTransaction);

		return range;
	}

	/// Deserializes \a cacheEntryInfo into a state model.
	template<typename TSerializer, typename TKey>
	auto DeserializeCacheEntryInfo(const model::CacheEntryInfo<TKey>& cacheEntryInfo) {
		RawBuffer buffer{ cacheEntryInfo.DataPtr(), cacheEntryInfo.DataSize };
		io::BufferInputStreamAdapter<RawBuffer> input(buffer);
		return TSerializer::Load(input);
	}

	/// Audits \a transactions by querying a connection from \a connections and forwarding to \a auditFunc.
	template<typename TTraits, typename TTransactions, typename TAuditFunc>
	bool AuditTransactions(const NetworkConnections& connections, const TTransactions& transactions, TAuditFunc auditFunc) {
		auto pair = connections.pickOne();
		auto pDiagnosticApi = extensions::CreateRemoteDiagnosticApi(*pair.io());

		auto infos = TTraits::GetInfos(*pDiagnosticApi, transactions);
		if (infos.size() != transactions.size()) {
			CATAPULT_LOG(error) << "unexpected size mismatch of returned infos vs sent transactions";
			return false;
		}

		typename TTraits::IdToNameMap idToName;
		for (const auto& pTransaction : transactions)
			idToName.emplace(TTraits::GetId(*pTransaction), TTraits::GetName(*pTransaction));

		bool hasFailure = false;
		for (const auto& info : infos) {
			auto name = idToName.at(info.Id);
			if (!info.HasData()) {
				CATAPULT_LOG(error) << "artifact did not make it into the chain: " << idToName.at(info.Id);
				hasFailure = true;
			}

			if (info.IsTooLarge()) {
				CATAPULT_LOG(error) << "size of artifact is too huge: " << idToName.at(info.Id);
				hasFailure = true;
			}

			if (!auditFunc(info, name))
				hasFailure = true;
		}

		return !hasFailure;
	}

	/// Sender that sends artifacts to a network.
	class ArtifactSender final {
	public:
		/// Creates a sender for the network with \a nodes that waits \a numBlocksToWait blocks after sending.
		explicit ArtifactSender(const std::vector<ionet::Node>& nodes, size_t numBlocksToWait)
				: m_connections(nodes)
				, m_numBlocksToWait(numBlocksToWait) {
			m_connections.connectAll().get();
		}

	public:
		/// Gets the network connections.
		NetworkConnections& connections() {
			return m_connections;
		}

	private:
		template<typename TTransactions>
		void sendTransactions(const TTransactions& transactions) {
			auto ioPair = m_connections.pickOne();
			if (!ioPair)
				CATAPULT_THROW_RUNTIME_ERROR("no connection available, setting up artifacts failed");

			CATAPULT_LOG(info) << "creating " << transactions.size() << " artifacts";
			auto transactionsPayload = ionet::PacketPayloadFactory::FromEntities(ionet::PacketType::Push_Transactions, transactions);
			ioPair.io()->write(transactionsPayload, [](auto code) {
				if (ionet::SocketOperationCode::Success == code)
					CATAPULT_LOG(info) << "write succeeded";
				else
					CATAPULT_THROW_RUNTIME_ERROR_1("write failed, result: ", code);
			});
		}

	public:
		/// Sends \a transactions to the network and waits for a configured number of blocks to be accepted.
		template<typename TTransactions>
		void sendAndWait(const TTransactions& transactions) {
			sendTransactions(transactions);

			// wait for all transactions to get included in a block
			if (!WaitForBlocks(m_connections, m_numBlocksToWait))
				CATAPULT_THROW_RUNTIME_ERROR("something went wrong while waiting for blocks to get harvested");
		}

	private:
		NetworkConnections m_connections;
		size_t m_numBlocksToWait;
	};
}}

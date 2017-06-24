#pragma once
#include "catapult/ionet/PacketEntityUtils.h"
#include "catapult/model/TransactionPlugin.h"
#include "catapult/utils/Logging.h"
#include <functional>

namespace catapult { namespace handlers {

	/// Creates a push handler that forwards a received entity range to \a rangeHandler
	/// given a \a registry composed of supported transaction types.
	template<typename TEntity>
	auto CreatePushEntityHandler(
			const model::TransactionRegistry& registry,
			const std::function<void (model::EntityRange<TEntity>&&)>& rangeHandler) {
		return [rangeHandler, &registry](const ionet::Packet& packet, const auto&) -> void {
			auto range = ionet::ExtractEntitiesFromPacket<TEntity>(packet, [&registry](const auto& entity) {
				return IsSizeValid(entity, registry);
			});
			if (range.empty()) {
				CATAPULT_LOG(warning) << "rejecting empty range (packet type = " << packet.Type << ", size " << packet.Size << ")";
				return;
			}

			CATAPULT_LOG(trace) << "received valid packet (type = " << packet.Type << ") with size " << packet.Size;
			rangeHandler(std::move(range));
		};
	}
}}

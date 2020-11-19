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
#include "catapult/io/BlockStorageCache.h"
#include "catapult/ionet/PacketHandlers.h"

namespace catapult { namespace handlers {

	/// Information about a height request.
	template<typename TRequest>
	struct HeightRequestInfo {
	public:
		/// Creates a height request info.
		constexpr HeightRequestInfo() : pRequest(nullptr)
		{}

	public:
		/// Current chain height.
		Height ChainHeight;

		/// Normalized request height.
		/// \note This is only useful when zero height is allowed.
		Height NormalizedRequestHeight;

		/// Coerced request.
		const TRequest* pRequest;

	public:
		/// Gets the number of remaining blocks in the chain.
		uint32_t numAvailableBlocks() const {
			return static_cast<uint32_t>((ChainHeight - NormalizedRequestHeight).unwrap() + 1);
		}
	};

	/// Helper for processing height requests.
	template<typename TRequest>
	class HeightRequestProcessor {
	private:
		static auto CreateResponsePacket(uint32_t payloadSize) {
			auto pPacket = ionet::CreateSharedPacket<ionet::Packet>(payloadSize);
			pPacket->Type = TRequest::Packet_Type;
			return pPacket;
		}

	public:
		/// Processes a height request (\a packet) using \a handlerContext and \a storage.
		/// Allows zero height requests if and only if \a shouldAllowZeroHeight is \c true.
		static HeightRequestInfo<TRequest> Process(
				const io::BlockStorageView& storage,
				const ionet::Packet& packet,
				ionet::ServerPacketHandlerContext& handlerContext,
				bool shouldAllowZeroHeight) {
			const auto* pRequest = ionet::CoercePacket<TRequest>(&packet);
			if (!pRequest)
				return HeightRequestInfo<TRequest>();

			HeightRequestInfo<TRequest> info;
			info.ChainHeight = storage.chainHeight();
			CATAPULT_LOG(trace) << "local height = " << info.ChainHeight << ", request height = " << pRequest->Height;
			if (info.ChainHeight < pRequest->Height || (!shouldAllowZeroHeight && Height(0) == pRequest->Height)) {
				handlerContext.response(ionet::PacketPayload(CreateResponsePacket(0)));
				return HeightRequestInfo<TRequest>();
			}

			info.NormalizedRequestHeight = Height(0) == pRequest->Height ? info.ChainHeight : pRequest->Height;
			info.pRequest = pRequest;
			return info;
		}
	};
}}

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
#include "HandlerTypes.h"
#include "catapult/ionet/PacketHandlers.h"
#include "catapult/model/ChainScore.h"
#include "catapult/model/RangeTypes.h"

namespace catapult { namespace io { class BlockStorageCache; } }

namespace catapult { namespace handlers {

	/// Registers a push block handler in \a handlers that validates a block and, if valid, forwards it to
	/// \a blockRangeHandler given a transaction \a registry composed of known transactions.
	void RegisterPushBlockHandler(
			ionet::ServerPacketHandlers& handlers,
			const model::TransactionRegistry& registry,
			const BlockRangeHandler& blockRangeHandler);

	/// Registers a pull block handler in \a handlers that responds with a block in \a storage.
	void RegisterPullBlockHandler(ionet::ServerPacketHandlers& handlers, const io::BlockStorageCache& storage);

	/// Registers a chain statistics handler in \a handlers that responds with the height of the chain in \a storage,
	/// the score returned by \a chainScoreSupplier and the finalized height returned by \a finalizedHeightSupplier.
	void RegisterChainStatisticsHandler(
			ionet::ServerPacketHandlers& handlers,
			const io::BlockStorageCache& storage,
			const model::ChainScoreSupplier& chainScoreSupplier,
			const supplier<Height>& finalizedHeightSupplier);

	/// Registers a block hashes handler in \a handlers that responds with at most \a maxHashes hashes in \a storage.
	void RegisterBlockHashesHandler(ionet::ServerPacketHandlers& handlers, const io::BlockStorageCache& storage, uint32_t maxHashes);

	/// Configuration for pull blocks handler.
	struct PullBlocksHandlerConfiguration {
		/// Maximum blocks to return.
		uint32_t MaxBlocks;

		/// Maximum response bytes.
		uint32_t MaxResponseBytes;
	};

	/// Registers a pull blocks handler in \a handlers that responds with blocks from \a storage according to behavior
	/// specified in \a config.
	void RegisterPullBlocksHandler(
			ionet::ServerPacketHandlers& handlers,
			const io::BlockStorageCache& storage,
			const PullBlocksHandlerConfiguration& config);
}}

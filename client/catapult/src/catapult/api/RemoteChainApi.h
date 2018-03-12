#pragma once
#include "ChainApi.h"

namespace catapult {
	namespace ionet { class PacketIo; }
	namespace model { class TransactionRegistry; }
}

namespace catapult { namespace api {

	/// Options for a blocks-from request.
	struct BlocksFromOptions {
		/// Creates blocks-from options.
		BlocksFromOptions() : BlocksFromOptions(0, 0)
		{}

		/// Creates blocks-from options from a number of requested blocks (\a numBlocks)
		/// and a number of requested bytes (\a numBytes).
		BlocksFromOptions(uint32_t numBlocks, uint32_t numBytes)
				: NumBlocks(numBlocks)
				, NumBytes(numBytes)
		{}

		/// The requested number of blocks.
		uint32_t NumBlocks;

		/// The requested number of bytes.
		uint32_t NumBytes;
	};

	/// An api for retrieving chain information from a remote node.
	class RemoteChainApi : public ChainApi {
	public:
		/// Gets the last block.
		virtual thread::future<std::shared_ptr<const model::Block>> blockLast() const = 0;

		/// Gets the block at \a height.
		virtual thread::future<std::shared_ptr<const model::Block>> blockAt(Height height) const = 0;

		/// Gets the blocks starting at \a height with the specified \a options.
		/// \note An empty range will be returned if remote chain height is less than \a height.
		virtual thread::future<model::BlockRange> blocksFrom(Height height, const BlocksFromOptions& options) const = 0;
	};

	/// Creates a chain api for interacting with a remote node with the specified \a io.
	std::unique_ptr<ChainApi> CreateRemoteChainApiWithoutRegistry(ionet::PacketIo& io);

	/// Creates a chain api for interacting with a remote node with the specified \a io
	/// and transaction \a registry composed of supported transactions.
	std::unique_ptr<RemoteChainApi> CreateRemoteChainApi(ionet::PacketIo& io, const model::TransactionRegistry& registry);
}}

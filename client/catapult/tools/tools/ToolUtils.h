#pragma once
#include "catapult/types.h"
#include <memory>

namespace catapult {
	namespace crypto { class KeyPair; }
	namespace ionet {
		struct Node;
		class PacketIo;
	}
	namespace model { class TransactionRegistry; }
	namespace thread {
		template<typename T>
		class future;
		class IoServiceThreadPool;
	}
}

namespace catapult { namespace tools {
	/// Creates a started thread pool with \a numThreads threads.
	std::shared_ptr<thread::IoServiceThreadPool> CreateStartedThreadPool(uint32_t numThreads = 1);

	/// Future that returns a packet io shared pointer.
	using PacketIoFuture = thread::future<std::shared_ptr<ionet::PacketIo>>;

	/// Connects to localhost having \a serverPublicKey as \a clientKeyPair using \a pPool.
	PacketIoFuture ConnectToLocalNode(
			const crypto::KeyPair& clientKeyPair,
			const Key& serverPublicKey,
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool);

	/// Connects to \a node as \a clientKeyPair using \a pPool.
	PacketIoFuture ConnectToNode(
			const crypto::KeyPair& clientKeyPair,
			const ionet::Node& node,
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool);

	/// Creates a default transaction registry with a single registered transaction.
	std::unique_ptr<model::TransactionRegistry> CreateDefaultTransactionRegistry();
}}

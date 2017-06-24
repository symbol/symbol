#include "catapult/local/p2p/LocalNode.h"
#include "catapult/server/ServerMain.h"
#include "catapult/thread/MultiServicePool.h"

int main(int argc, const char** argv) {
	using namespace catapult;
	return server::ServerMain(argc, argv, [](auto&& config, const auto& keyPair) {
		auto numServicePoolThreads = thread::MultiServicePool::DefaultPoolConcurrency();
		return local::p2p::CreateLocalNode(
				keyPair,
				std::move(config),
				std::make_unique<thread::MultiServicePool>(numServicePoolThreads, "p2p"));
	});
}

#pragma once
#include "catapult/extensions/BlockChainStorage.h"
#include <memory>

namespace catapult { namespace filechain {

	/// Creates a block chain storage for saving and loading state to and from files.
	std::unique_ptr<extensions::BlockChainStorage> CreateFileBlockChainStorage();
}}

#pragma once
#include "ServerHooks.h"

namespace catapult { namespace extensions {

	/// Creates a block push entity callback from \a hooks that only pushes when synced.
	BlockRangeConsumerFunc CreateBlockPushEntityCallback(const ServerHooks& hooks);

	/// Creates a transaction push entity callback from \a hooks that only pushes when synced.
	TransactionRangeConsumerFunc CreateTransactionPushEntityCallback(const ServerHooks& hooks);
}}

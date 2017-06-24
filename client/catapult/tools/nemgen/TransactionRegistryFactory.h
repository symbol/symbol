#pragma once
#include "catapult/model/TransactionPlugin.h"

namespace catapult { namespace tools { namespace nemgen {

	/// Creates a transaction registry that has basic transaction types registered.
	model::TransactionRegistry CreateTransactionRegistry();
}}}

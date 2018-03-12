#pragma once
#include "AccountState.h"
#include "catapult/model/AccountInfo.h"

namespace catapult { namespace state {

	/// Creates an account state from an account \a info.
	AccountState ToAccountState(const model::AccountInfo& info);

	/// Creates an account info from an account state (\a accountState).
	std::unique_ptr<model::AccountInfo> ToAccountInfo(const AccountState& accountState);
}}


#pragma once
#include "MapperInclude.h"
#include <functional>

namespace catapult { namespace state { struct AccountState; } }

namespace catapult { namespace mongo { namespace mappers {

	/// Prototype for creating account states around an address and a height.
	using AccountStateFactory = std::function<state::AccountState& (const Address&, Height)>;

	/// Maps an account state (\a accountState) to the corresponding db model value.
	bsoncxx::document::value ToDbModel(const state::AccountState& accountState);

	/// Creates account state using \a accountStateFactory out of \a document.
	void ToAccountState(const bsoncxx::document::view& document, const AccountStateFactory& accountStateFactory);
}}}

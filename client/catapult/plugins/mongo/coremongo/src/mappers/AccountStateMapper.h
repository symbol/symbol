#pragma once
#include "catapult/types.h"
#include <functional>
#include <memory>

namespace catapult { namespace state { struct AccountState; } }

namespace bsoncxx {
	inline namespace v_noabi {
		namespace document {
			class value;
			class view;
		}
	}
}

namespace catapult { namespace mongo { namespace mappers {

	/// Prototype for creating account states around an address and a height.
	using AccountStateFactory = std::function<std::shared_ptr<state::AccountState> (const Address&, Height)>;

	/// Maps an account state (\a accountState) to the corresponding db model entity.
	bsoncxx::document::value ToDbModel(const state::AccountState& accountState);

	/// Creates account state using \a accountStateFactory out of \a document.
	void ToAccountState(const bsoncxx::document::view& document, const AccountStateFactory& accountStateFactory);
}}}

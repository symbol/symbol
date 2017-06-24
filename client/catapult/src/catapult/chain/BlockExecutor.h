#pragma once
#include "catapult/model/Elements.h"
#include "catapult/observers/ObserverTypes.h"
#include <memory>

namespace catapult { namespace model { struct Block; } }

namespace catapult { namespace chain {

	/// Executes \a blockElement while updating \a state using \a observer.
	void ExecuteBlock(
			const model::BlockElement& blockElement,
			const observers::EntityObserver& observer,
			const observers::ObserverState& state);

	/// Rollbacks \a blockElement while updating \a state using \a observer.
	void RollbackBlock(
			const model::BlockElement& blockElement,
			const observers::EntityObserver& observer,
			const observers::ObserverState& state);
}}

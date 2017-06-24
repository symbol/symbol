#pragma once
#include <functional>
#include <memory>

namespace catapult { namespace cache { class MemoryUtCacheView; } }

namespace catapult { namespace local { namespace api {

	/// A function prototype for creating an unconfirmed transactions cache view.
	using UnconfirmedTransactionsCacheViewProvider = std::function<cache::MemoryUtCacheView ()>;
}}}

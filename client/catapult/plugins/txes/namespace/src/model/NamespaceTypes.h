#pragma once
#include <stdint.h>

namespace catapult { namespace model {

	/// The type of namespace being registered.
	enum class NamespaceType : uint8_t {
		/// A root namespace.
		Root,
		/// A child namespace.
		Child
	};
}}

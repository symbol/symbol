#pragma once
#include "catapult/exceptions.h"
#include "catapult/types.h"

namespace catapult { namespace state {

	/// The lifetime of a namespace.
	struct NamespaceLifetime {
	public:
		/// Creates a lifetime composed of a \a start height and an \a end height.
		explicit NamespaceLifetime(Height start, Height end)
				: Start(start)
				, End(end) {
			if (start >= end)
				CATAPULT_THROW_INVALID_ARGUMENT("namespace lifetime must be positive");
		}

	public:
		bool isActive(Height height) const {
			return height >= Start && height < End;
		}

	public:
		/// Returns \c true if this NamespaceLifetime is equal to \a rhs.
		bool operator==(const NamespaceLifetime& rhs) const {
			return Start == rhs.Start && End == rhs.End;
		}

		/// Returns \c true if this NamespaceLifetime is not equal to \a rhs.
		bool operator!=(const NamespaceLifetime& rhs) const {
			return !(*this == rhs);
		}

	public:
		/// The start height.
		Height Start;

		/// The end height.
		Height End;
	};
}}

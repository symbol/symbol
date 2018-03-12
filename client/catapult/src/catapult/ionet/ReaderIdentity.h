#pragma once
#include "catapult/types.h"
#include <string>

namespace catapult { namespace ionet {

	/// Identifying information about a reader.
	struct ReaderIdentity {
		/// The reader's uniquely identifying public key.
		Key PublicKey;

		/// The reader's network host.
		std::string Host;
	};

	/// Insertion operator for outputting \a identity to \a out.
	std::ostream& operator<<(std::ostream& out, const ReaderIdentity& identity);
}}

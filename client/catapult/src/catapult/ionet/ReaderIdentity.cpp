#include "ReaderIdentity.h"
#include "catapult/utils/HexFormatter.h"

namespace catapult { namespace ionet {

	std::ostream& operator<<(std::ostream& out, const ReaderIdentity& identity) {
		out << "reader (" << utils::HexFormat(identity.PublicKey) << " @ " << identity.Host << ")";
		return out;
	}
}}

#pragma once
#include "SupplementalData.h"

namespace catapult {
	namespace io {
		class InputStream;
		class OutputStream;
	}
}

namespace catapult { namespace cache {
	/// Saves \a supplementalData and \a chainHeight to \a output.
	void SaveSupplementalData(const SupplementalData& supplementalData, Height chainHeight, io::OutputStream& output);

	/// Loads \a supplementalData and \a chainHeight from \a input.
	void LoadSupplementalData(io::InputStream& input, SupplementalData& supplementalData, Height& chainHeight);
}}

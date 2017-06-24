#pragma once
#include "plugins/txes/namespace/src/state/MosaicDescriptor.h"

namespace bsoncxx {
	inline namespace v_noabi {
		namespace document {
			class value;
			class view;
		}
	}
}

namespace catapult { namespace mongo { namespace mappers {

	/// Maps a mosaic \a descriptor to the corresponding db model entity.
	bsoncxx::document::value ToDbModel(const state::MosaicDescriptor& descriptor);

	/// Maps a database \a document to the corresponding model entity.
	state::MosaicDescriptor ToMosaicDescriptor(const bsoncxx::document::view& document);
}}}

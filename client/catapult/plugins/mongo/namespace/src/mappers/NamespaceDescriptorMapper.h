#pragma once
#include "plugins/txes/namespace/src/state/NamespaceDescriptor.h"

namespace bsoncxx {
	inline namespace v_noabi {
		namespace document {
			class value;
			class view;
		}
	}
}

namespace catapult { namespace mongo { namespace mappers {
	
	/// Maps a namespace \a descriptor to the corresponding db model entity.
	bsoncxx::document::value ToDbModel(const state::NamespaceDescriptor& descriptor);

	/// Maps a database \a document to the corresponding model entity.
	state::NamespaceDescriptor ToNamespaceDescriptor(const bsoncxx::document::view& document);
}}}

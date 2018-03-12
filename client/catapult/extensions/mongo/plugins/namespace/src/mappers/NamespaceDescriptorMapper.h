#pragma once
#include "NamespaceDescriptor.h"
#include "mongo/src/mappers/MapperInclude.h"

namespace catapult { namespace mongo { namespace plugins {

	/// Maps a namespace \a descriptor to the corresponding db model value.
	bsoncxx::document::value ToDbModel(const NamespaceDescriptor& descriptor);

	/// Maps a database \a document to the corresponding model value.
	NamespaceDescriptor ToNamespaceDescriptor(const bsoncxx::document::view& document);
}}}

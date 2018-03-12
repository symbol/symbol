#pragma once
#include "MosaicDescriptor.h"
#include "mongo/src/mappers/MapperInclude.h"

namespace catapult { namespace mongo { namespace plugins {

	/// Maps a mosaic \a descriptor to the corresponding db model value.
	bsoncxx::document::value ToDbModel(const MosaicDescriptor& descriptor);

	/// Maps a database \a document to the corresponding model value.
	MosaicDescriptor ToMosaicDescriptor(const bsoncxx::document::view& document);
}}}

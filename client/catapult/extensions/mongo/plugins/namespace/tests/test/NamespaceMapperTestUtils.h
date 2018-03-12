#pragma once
#include <bsoncxx/json.hpp>

namespace catapult {
	namespace mongo {
		namespace plugins {
			struct MosaicDescriptor;
			struct NamespaceDescriptor;
		}
	}
}

namespace catapult { namespace test {

	/// Verifies that the metadata in \a descriptor matches mosaic metadata (\a dbMetadata) in db.
	void AssertEqualMosaicMetadata(const mongo::plugins::MosaicDescriptor& descriptor, const bsoncxx::document::view& dbMetadata);

	/// Verifies that db mosaic (\a dbMosaic) and model mosaic \a descriptor are equivalent.
	void AssertEqualMosaicData(const mongo::plugins::MosaicDescriptor& descriptor, const bsoncxx::document::view& dbMosaic);

	/// Verifies that the metadata in \a descriptor matches namespace metadata (\a dbMetadata) in db.
	void AssertEqualNamespaceMetadata(const mongo::plugins::NamespaceDescriptor& descriptor, const bsoncxx::document::view& dbMetadata);

	/// Verifies that db namespace (\a dbNamespace) and model namespace \a descriptor are equivalent.
	void AssertEqualNamespaceData(const mongo::plugins::NamespaceDescriptor& descriptor, const bsoncxx::document::view& dbNamespace);
}}

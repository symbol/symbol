#pragma once

namespace bsoncxx { inline namespace v_noabi { namespace document { class view; } } }

namespace catapult {
	namespace state {
		struct MosaicDescriptor;
		struct NamespaceDescriptor;
	}
}

namespace catapult { namespace mongo { namespace test {

	/// Verifies that the metadata in \a descriptor matches mosaic metadata (\a dbMetadata) in db.
	void AssertEqualMosaicMetadata(const state::MosaicDescriptor& descriptor, const bsoncxx::document::view& dbMetadata);

	/// Verifies that db mosaic (\a dbMosaic) and model mosaic \a descriptor are equivalent.
	void AssertEqualMosaicData(const state::MosaicDescriptor& descriptor, const bsoncxx::document::view& dbMosaic);

	/// Verifies that the metadata in \a descriptor matches namespace metadata (\a dbMetadata) in db.
	void AssertEqualNamespaceMetadata(const state::NamespaceDescriptor& descriptor, const bsoncxx::document::view& dbMetadata);

	/// Verifies that db namespace (\a dbNamespace) and model namespace \a descriptor are equivalent.
	void AssertEqualNamespaceData(const state::NamespaceDescriptor& descriptor, const bsoncxx::document::view& dbNamespace);
}}}

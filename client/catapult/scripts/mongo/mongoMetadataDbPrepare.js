(function prepareMetadataCollections() {
	db.createCollection('metadata');
	db.metadata.createIndex({ 'metadataEntry.compositeHash': 1 }, { unique: true });
	db.metadata.createIndex({ 'metadataEntry.sourceAddress': 1, 'metadataEntry.metadataType': 1, 'metadataEntry.scopedMetadataKey': 1 });
	db.metadata.createIndex({ 'metadataEntry.targetAddress': 1, 'metadataEntry.metadataType': 1, 'metadataEntry.scopedMetadataKey': 1 });
})();

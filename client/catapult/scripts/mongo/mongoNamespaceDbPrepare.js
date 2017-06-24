(function prepareNamespaceCollections() {
	db.createCollection('namespaces');
	db.namespaces.createIndex({ 'namespace.level0': 1, 'meta.index': 1, 'namespace.depth': 1 }, { unique: true });
	db.namespaces.createIndex({ 'meta.active': true, 'namespace.level0': 1, 'namespace.depth': 1 }, { unique: true });
	db.namespaces.createIndex({ 'meta.active': true, 'namespace.level1': 1, 'namespace.depth': 1 });
	db.namespaces.createIndex({ 'meta.active': true, 'namespace.level2': 1, 'namespace.depth': 1 });
	db.namespaces.createIndex({ 'meta.active': true, 'namespace.owner': 1 });

	db.createCollection('mosaics');
	db.mosaics.createIndex({ 'mosaic.namespaceId': 1, 'mosaic.mosaicId': 1, 'meta.index': 1 }, { unique: true });
	db.mosaics.createIndex({ 'meta.active': true, 'mosaic.mosaicId': 1 }, { unique: true });
	db.mosaics.createIndex({ 'meta.active': true, 'mosaic.definition.owner': 1 });

	db.namespaces.getIndexes();
	db.mosaics.getIndexes();
})();

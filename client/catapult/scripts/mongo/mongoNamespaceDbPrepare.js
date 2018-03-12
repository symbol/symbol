(function prepareNamespaceCollections() {
	db.createCollection('namespaces');
	db.namespaces.createIndex({ 'namespace.level0': 1, 'meta.index': 1, 'namespace.depth': 1 });
	db.namespaces.createIndex({ 'meta.active': -1, 'meta.index': 1, 'namespace.level0': 1, 'namespace.depth': 1 });
	db.namespaces.createIndex({ 'meta.active': -1, 'namespace.level1': 1, 'namespace.depth': 1 });
	db.namespaces.createIndex({ 'meta.active': -1, 'namespace.level2': 1, 'namespace.depth': 1 });
	db.namespaces.createIndex({ 'meta.active': -1, 'namespace.owner': 1 });
	db.namespaces.createIndex({ 'meta.active': -1, 'namespace.ownerAddress': 1 });

	db.createCollection('mosaics');
	db.mosaics.createIndex({ 'mosaic.namespaceId': 1, 'mosaic.mosaicId': 1, 'meta.index': 1 }, { unique: true });
	db.mosaics.createIndex({ 'meta.active': -1, 'meta.index': 1, 'mosaic.mosaicId': 1 }, { unique: true });
	db.mosaics.createIndex({ 'meta.active': -1, 'mosaic.definition.owner': 1 });

	db.namespaces.getIndexes();
	db.mosaics.getIndexes();
})();

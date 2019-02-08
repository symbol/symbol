(function prepareMosaicCollections() {
	db.createCollection('mosaics');
	db.mosaics.createIndex({ 'mosaic.mosaicId': 1 }, { unique: true });
	db.mosaics.createIndex({ 'mosaic.definition.owner': 1 });

	db.mosaics.getIndexes();
})();

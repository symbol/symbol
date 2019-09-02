(function prepareMosaicCollections() {
	db.createCollection('mosaics');
	db.mosaics.createIndex({ 'mosaic.id': 1 }, { unique: true });
	db.mosaics.createIndex({ 'mosaic.ownerPublicKey': 1 });
})();

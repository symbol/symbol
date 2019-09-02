(function prepareRestrictionMosaicCollections() {
	db.createCollection('mosaicRestrictions');
	db.mosaicRestrictions.createIndex({ 'mosaicRestrictionEntry.compositeHash': 1 }, { unique: true });
})();

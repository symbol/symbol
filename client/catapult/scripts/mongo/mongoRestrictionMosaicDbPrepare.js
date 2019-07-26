(function prepareRestrictionMosaicCollection() {
	db.createCollection('mosaicRestrictions');
	db.mosaicRestrictions.createIndex({ 'mosaicRestrictionEntry.compositeHash': 1 }, { unique: true });

	db.mosaicRestrictions.getIndexes();
})();

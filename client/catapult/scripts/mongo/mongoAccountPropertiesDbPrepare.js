(function prepareAccountPropertiesCollection() {
	db.createCollection('accountProperties');
	db.accountProperties.createIndex({ 'accountProperties.address': 1 }, { unique: true });

	db.accountProperties.getIndexes();
})();

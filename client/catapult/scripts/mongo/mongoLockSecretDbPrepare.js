(function prepareSecretLockCollections() {
	db.createCollection('secretLocks');
	db.secretLocks.createIndex({ 'lock.compositeHash': 1 }, { unique: true });
	db.secretLocks.createIndex({ 'lock.ownerAddress': 1 });
})();

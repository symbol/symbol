(function prepareSecretLockCollections() {
	db.createCollection('secretLocks');
	db.secretLocks.createIndex({ 'lock.compositeHash': 1 }, { unique: true });
	db.secretLocks.createIndex({ 'lock.senderPublicKey': 1 });
	db.secretLocks.createIndex({ 'lock.senderAddress': 1 });
})();

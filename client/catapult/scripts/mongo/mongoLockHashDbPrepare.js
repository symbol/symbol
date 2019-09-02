(function prepareHashLockCollections() {
	db.createCollection('hashLocks');
	db.hashLocks.createIndex({ 'lock.hash': 1 }, { unique: true });
	db.hashLocks.createIndex({ 'lock.senderPublicKey': 1 });
	db.hashLocks.createIndex({ 'lock.senderAddress': 1 });
})();

(function prepareHashLockCollections() {
	db.createCollection('hashLocks');
	db.hashLocks.createIndex({ 'lock.hash': 1 }, { unique: true });
	db.hashLocks.createIndex({ 'lock.ownerAddress': 1 });
})();

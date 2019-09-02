(function prepareMultisigCollections() {
	db.createCollection('multisigs');
	db.multisigs.createIndex({ 'multisig.accountPublicKey': 1 }, { unique: true });
	db.multisigs.createIndex({ 'multisig.accountAddress': 1 }, { unique: true });
})();

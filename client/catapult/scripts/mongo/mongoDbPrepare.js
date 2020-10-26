(function prepareDbConfiguration() {
	db.setProfilingLevel(1, 100); // log slow queries
})();

(function prepareDbCollections() {
	function makeSparse(propertyName) {
		return { partialFilterExpression: { [propertyName]: { $exists: true } } };
	}

	function addCommonTransactionIndexes(collection) {
		collection.createIndex({ 'transaction.signerPublicKey': 1, _id: -1 });
		collection.createIndex({ 'transaction.recipientAddress': 1, _id: -1 });
		collection.createIndex({ 'meta.hash': 1 }, Object.assign({ unique: true }, makeSparse('meta.hash')));
		collection.createIndex({ 'meta.addresses': 1 }, makeSparse('meta.addresses'));
		collection.createIndex({ 'meta.aggregateId': 1 }, makeSparse('meta.aggregateId'));
	}

	function addTransactionCollection(collectionName) {
		db.createCollection(collectionName);
		addCommonTransactionIndexes(db[collectionName]);
		db[collectionName].createIndex({ 'meta.aggregateHash': 1 }, makeSparse('meta.aggregateHash'));
	}

	db.createCollection('blocks');
	db.blocks.createIndex({ 'block.signerPublicKey': 1 });
	db.blocks.createIndex({ 'block.timestamp': -1 }, { unique: true });
	db.blocks.createIndex({ 'block.height': -1 }, { unique: true });
	db.blocks.createIndex({ 'block.signerPublicKey': 1, 'block.height': -1 }, { unique: true });
	db.blocks.createIndex({ 'block.beneficiaryAddress': 1, 'block.height': -1 }, { unique: true });

	db.createCollection('finalizedBlocks');
	db.finalizedBlocks.createIndex({ 'block.finalizationEpoch': -1 });
	db.finalizedBlocks.createIndex({ 'block.height': -1 });
	db.finalizedBlocks.createIndex({ 'block.finalizationEpoch': -1, 'block.finalizationPoint': -1 }, { unique: true });

	db.createCollection('transactions');
	addCommonTransactionIndexes(db.transactions);
	db.transactions.createIndex({ 'meta.height': -1 });
	db.transactions.createIndex({ 'transaction.deadline': -1 });
	db.transactions.createIndex({ 'transaction.cosignatures.signerPublicKey': 1 }, makeSparse('transaction.cosignatures.signerPublicKey'));
	db.transactions.createIndex({ 'transaction.id': 1, 'transaction.type': 1 }, makeSparse('transaction.id'));

	db.createCollection('transactionStatements');
	db.transactionStatements.createIndex(
		{ 'statement.height': 1, 'statement.source.primaryId': 1, 'statement.source.secondaryId': 1 },
		{ unique: true });

	['addressResolutionStatements', 'mosaicResolutionStatements'].forEach(collectionName => {
		db.createCollection(collectionName);
		db[collectionName].createIndex({ 'statement.height': 1, 'statement.unresolved': 1 }, { unique: true });
	});

	db.createCollection('accounts');
	db.accounts.createIndex({ 'account.publicKey': 1 }); // cannot be unique because zeroed public keys are stored
	db.accounts.createIndex({ 'account.address': 1 }, { unique: true });

	['unconfirmedTransactions', 'partialTransactions'].forEach(addTransactionCollection);

	db.createCollection('transactionStatuses');
	db.transactionStatuses.createIndex({ 'status.hash': 1 }, { unique: true });
	db.transactionStatuses.createIndex({ 'status.deadline': -1 });
})();

(function preparePluginDbCollections() {
	const pluginNames = [
		'LockHash', 'LockSecret', 'Metadata', 'Mosaic', 'Multisig', 'Namespace', 'RestrictionAccount', 'RestrictionMosaic'
	];
	pluginNames.forEach(pluginName => {
		print(`Loading ${pluginName}`);
		load(`mongo${pluginName}DbPrepare.js`);
	});
})();

(function printCollectionIndexes() {
	db.getCollectionNames().forEach(collectionName => {
		print(`===== ${collectionName} INDEXES =====`);
		db[collectionName].getIndexes().forEach(index => {
			printjson(index.key);
		});
	});
})();

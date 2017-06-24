db.createCollection('blocks');
db.blocks.createIndex({ 'block.signer': 1 });
db.blocks.createIndex({ 'block.timestamp': -1 }, { unique: true });
db.blocks.createIndex({ 'block.height': -1 }, { unique: true });
db.blocks.createIndex({ 'block.signer': 1, 'block.height': -1 }, { unique: true });

db.createCollection('transactions');
db.transactions.createIndex({ 'transaction.deadline': -1 });
db.transactions.createIndex({ 'transaction.signer': 1, _id: -1 });
db.transactions.createIndex({ 'transaction.recipient': 1, _id: -1 });
db.transactions.createIndex({ 'meta.height': -1 });
db.transactions.createIndex({ 'meta.aggregateId': 1 }, { sparse: true });

db.createCollection('accounts');
db.accounts.createIndex({ 'account.publicKey': 1 }); // cannot be unique because zeroed public keys are stored
db.accounts.createIndex({ 'account.address': 1 }, { unique: true });

db.createCollection('unconfirmedTransactions');
db.unconfirmedTransactions.createIndex({ 'meta.hash': 1 }, { unique: true, sparse: true });
db.unconfirmedTransactions.createIndex({ 'transaction.signer': 1, _id: -1 });
db.unconfirmedTransactions.createIndex({ 'transaction.recipient': 1, _id: -1 });
db.unconfirmedTransactions.createIndex({ 'meta.aggregateId': 1 }, { sparse: true });

db.blocks.getIndexes();
db.transactions.getIndexes();
db.accounts.getIndexes();
db.unconfirmedTransactions.getIndexes();

load("mongo/mongoMultisigDbPrepare.js")
load("mongo/mongoNamespaceDbPrepare.js")

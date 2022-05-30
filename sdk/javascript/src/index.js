const { BaseValue } = require('./BaseValue');
const { Bip32 } = require('./Bip32');
const { ByteArray } = require('./ByteArray');
const CryptoTypes = require('./CryptoTypes');
const { NetworkLocator } = require('./Network');
const NemFacade = require('./facade/NemFacade');
const SymbolFacade = require('./facade/SymbolFacade');
const NemKeyPair = require('./nem/KeyPair');
const NemNetwork = require('./nem/Network');
const NemTransactionFactory = require('./nem/TransactionFactory');
const NemModels = require('./nem/models');
const SymbolKeyPair = require('./symbol/KeyPair');
const SymbolMerkleHashBuilder = require('./symbol/MerkleHashBuilder');
const SymbolNetwork = require('./symbol/Network');
const SymbolTransactionFactory = require('./symbol/TransactionFactory');
const SymbolIdGenerator = require('./symbol/idGenerator');
const SymbolModels = require('./symbol/models');
const { hexToUint8, uint8ToHex } = require('./utils/converter');

module.exports = {
	BaseValue,
	Bip32,
	ByteArray,
	CryptoTypes,
	NetworkLocator,

	facade: {
		...NemFacade,
		...SymbolFacade
	},

	nem: {
		...NemModels, // must be before Network to promote Address from Network
		...NemKeyPair,
		...NemNetwork,
		...NemTransactionFactory
	},

	symbol: {
		...SymbolModels, // must be before Network to promote Address from Network
		...SymbolKeyPair,
		...SymbolMerkleHashBuilder,
		...SymbolNetwork,
		...SymbolTransactionFactory,
		...SymbolIdGenerator
	},

	utils: {
		hexToUint8,
		uint8ToHex
	}
};

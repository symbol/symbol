import BaseValue from './BaseValue.js';
import Bip32 from './Bip32.js';
import ByteArray from './ByteArray.js';
import * as CryptoTypes from './CryptoTypes.js';
import { NetworkLocator } from './Network.js';
import NemFacade from './facade/NemFacade.js';
import SymbolFacade from './facade/SymbolFacade.js';
import * as NemKeyPair from './nem/KeyPair.js';
import NemMessageEncoder from './nem/MessageEncoder.js';
import * as NemNetwork from './nem/Network.js';
import * as NemTransactionFactory from './nem/TransactionFactory.js';
import * as NemModels from './nem/models.js';
import * as SymbolKeyPair from './symbol/KeyPair.js';
import SymbolMessageEncoder from './symbol/MessageEncoder.js';
import * as SymbolNetwork from './symbol/Network.js';
import * as SymbolTransactionFactory from './symbol/TransactionFactory.js';
import * as SymbolIdGenerator from './symbol/idGenerator.js';
import * as SymbolMerkle from './symbol/merkle.js';
import * as SymbolMetadata from './symbol/metadata.js';
import * as SymbolModels from './symbol/models.js';
import { hexToUint8, uint8ToHex } from './utils/converter.js';

const sdk = {
	BaseValue,
	Bip32,
	ByteArray,
	...CryptoTypes,
	NetworkLocator,

	facade: {
		NemFacade,
		SymbolFacade
	},

	nem: {
		...NemModels, // must be before Network to promote Address from Network

		...NemKeyPair,
		MessageEncoder: NemMessageEncoder,
		...NemNetwork,
		NemTransactionFactory
	},

	symbol: {
		...SymbolModels, // must be before Network to promote Address from Network

		...SymbolKeyPair,
		MessageEncoder: SymbolMessageEncoder,
		...SymbolNetwork,
		SymbolTransactionFactory,

		...SymbolIdGenerator,
		...SymbolMerkle,
		...SymbolMetadata
	},

	utils: {
		hexToUint8,
		uint8ToHex
	}
};

export default sdk;

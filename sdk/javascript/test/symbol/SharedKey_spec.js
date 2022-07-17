const { SymbolFacade } = require('../../src/facade/SymbolFacade');
const { KeyPair } = require('../../src/symbol/KeyPair');
const { runBasicSharedKeyTests } = require('../test/sharedKeyTests');

describe('SharedKey (Symbol)', () => {
	runBasicSharedKeyTests({
		KeyPair,
		deriveSharedKey: SymbolFacade.deriveSharedKey
	});
});

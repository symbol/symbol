import SymbolFacade from '../../src/facade/SymbolFacade.js';
import { KeyPair } from '../../src/symbol/KeyPair.js';
import runBasicSharedKeyTests from '../test/sharedKeyTests.js';

describe('SharedKey (Symbol)', () => {
	runBasicSharedKeyTests({
		KeyPair,
		deriveSharedKey: SymbolFacade.deriveSharedKey
	});
});

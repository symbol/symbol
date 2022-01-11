const { Address } = require('../../src/symbol/Network');
const {
	generateMosaicId, generateNamespaceId, generateMosaicAliasId, isValidNamespaceName, generateNamespacePath
} = require('../../src/symbol/idGenerator');
const { expect } = require('chai');
const crypto = require('crypto');

const TEST_VECTORS = {
	uppercase: ['CAT.token', 'CAT.TOKEN', 'cat.TOKEN', 'cAt.ToKeN', 'CaT.tOkEn'],
	improperPart: ['alpha.bet@.zeta', 'a!pha.beta.zeta', 'alpha.beta.ze^a'],
	improperQualified: ['.', '..', '...', '.a', 'b.', 'a..b', '.a.b', 'b.a.']
};

describe('idGenerator', () => {
	// region generateMosaicId

	describe('generateMosaicId', () => {
		it('generates correct id', () => {
			// Act:
			const mosaicId = generateMosaicId(new Address('TATNE7Q5BITMUTRRN6IB4I7FLSDRDWZA37JGO5Q'), 812613930);

			// Assert:
			expect(mosaicId).to.equal(BigInt('0x570FB3ED9379624C'));
		});

		it('produces different ids given different addresses', () => {
			// Arrange:
			const address1 = new Address(crypto.randomBytes(Address.SIZE));
			const address2 = new Address(crypto.randomBytes(Address.SIZE));

			// Act:
			const mosaicId1 = generateMosaicId(address1, 812613930);
			const mosaicId2 = generateMosaicId(address2, 812613930);

			// Assert:
			expect(mosaicId2).to.not.equal(mosaicId1);
		});

		it('produces different ids given different nonces', () => {
			// Arrange:
			const address = new Address(crypto.randomBytes(Address.SIZE));

			// Act:
			const mosaicId1 = generateMosaicId(address, 812613930);
			const mosaicId2 = generateMosaicId(address, 812613931);

			// Assert:
			expect(mosaicId2).to.not.equal(mosaicId1);
		});

		it('clears high bit', () => {
			// Arrange:
			for (let i = 0; 1000 > i; ++i) {
				const address = new Address(crypto.randomBytes(Address.SIZE));

				// Act:
				const mosaicId = generateMosaicId(address, 812613930);

				// Assert:
				expect(mosaicId >> BigInt(63), `address: ${address}`).to.equal(BigInt(0));
			}
		});
	});

	// endregion

	// region generateNamespaceId

	describe('generateNamespaceId', () => {
		it('generates correct id', () => {
			// Act:
			const namespaceId = generateNamespaceId('symbol');

			// Assert:
			expect(namespaceId).to.equal(BigInt('0xA95F1F8A96159516'));
		});

		it('generates correct child id', () => {
			// Act:
			const namespaceId = generateNamespaceId('xym', BigInt('0xA95F1F8A96159516'));

			// Assert:
			expect(namespaceId).to.equal(BigInt('0xE74B99BA41F4AFEE'));
		});

		it('produces different ids given different names', () => {
			// Act:
			const namespaceId1 = generateNamespaceId('symbol');
			const namespaceId2 = generateNamespaceId('Symbol');

			// Assert:
			expect(namespaceId2).to.not.equal(namespaceId1);
		});

		it('produces different ids given different parents', () => {
			// Act:
			const namespaceId1 = generateNamespaceId('symbol', BigInt('0xA95F1F8A96159516'));
			const namespaceId2 = generateNamespaceId('symbol', BigInt('0xA95F1F8A96159517'));

			// Assert:
			expect(namespaceId2).to.not.equal(namespaceId1);
		});

		it('sets high bit', () => {
			// Arrange:
			for (let i = 0; 1000 > i; ++i) {
				// Act:
				const namespaceId = generateNamespaceId('symbol', BigInt(i));

				// Assert:
				expect(namespaceId >> BigInt(63), `i: ${i}`).to.equal(BigInt(1));
			}
		});
	});

	// endregion

	// region generateMosaicAliasId

	describe('generateMosaicAliasId', () => {
		it('generates correct id', () => {
			// Act:
			const mosaicId = generateMosaicAliasId('cat.token');

			// Assert:
			expect(mosaicId).to.equal(BigInt('0xA029E100621B2E33'));
		});

		it('supports multilevel mosaics', () => {
			// Act:
			const mosaicId = generateMosaicAliasId('foo.bar.baz.xyz');

			// Assert:
			const namespaceId = generateNamespaceId('baz', generateNamespaceId('bar', generateNamespaceId('foo')));
			const expectedMosaicId = generateNamespaceId('xyz', namespaceId);
			expect(mosaicId).to.equal(expectedMosaicId);
		});

		const assertRejected = names => {
			names.forEach(name => {
				expect(() => { generateMosaicAliasId(name); }, `name: ${name}`).to.throw('invalid part name');
			});
		};

		it('rejects uppercase characters', () => {
			assertRejected(TEST_VECTORS.uppercase);
		});

		it('rejects improper part names', () => {
			assertRejected(TEST_VECTORS.improperPart);
		});

		it('rejects improper qualified names', () => {
			assertRejected(TEST_VECTORS.improperQualified);
		});

		it('rejects empty string', () => {
			assertRejected(['']);
		});
	});

	// endregion

	// region generateNamespacePath

	describe('generateNamespacePath', () => {
		it('generates correct root id', () => {
			// Act:
			const path = generateNamespacePath('cat');

			// Assert:
			expect(path).to.deep.equal([BigInt('0xB1497F5FBA651B4F')]);
		});

		it('generates correct child id', () => {
			// Act:
			const path = generateNamespacePath('cat.token');

			// Assert:
			expect(path).to.deep.equal([BigInt('0xB1497F5FBA651B4F'), BigInt('0xA029E100621B2E33')]);
		});

		it('supports multilevel namespaces', () => {
			// Act:
			const path = generateNamespacePath('foo.bar.baz.xyz');

			// Assert:
			const expectedPath = [generateNamespaceId('foo')];
			['bar', 'baz', 'xyz'].forEach(name => {
				expectedPath.push(generateNamespaceId(name, expectedPath[expectedPath.length - 1]));
			});

			expect(path).to.deep.equal(expectedPath);
		});

		const assertRejected = names => {
			names.forEach(name => {
				expect(() => { generateNamespacePath(name); }, `name: ${name}`).to.throw('invalid part name');
			});
		};

		it('rejects uppercase characters', () => {
			assertRejected(TEST_VECTORS.uppercase);
		});

		it('rejects improper part names', () => {
			assertRejected(TEST_VECTORS.improperPart);
		});

		it('rejects improper qualified names', () => {
			assertRejected(TEST_VECTORS.improperQualified);
		});

		it('rejects empty string', () => {
			assertRejected(['']);
		});
	});

	// endregion

	// region isValidNamespaceName

	describe('isValidNamespaceName', () => {
		it('returns true when all characters are alphanumeric', () => {
			['a', 'be', 'cat', 'doom', '09az09', 'az09az'].forEach(name => {
				expect(isValidNamespaceName(name), `name: ${name}`).to.equal(true);
			});
		});

		it('returns true when name contains separator', () => {
			['al-ce', 'al_ce', 'alice-', 'alice_'].forEach(name => {
				expect(isValidNamespaceName(name), `name: ${name}`).to.equal(true);
			});
		});

		it('returns false when name starts with separator', () => {
			['-alice', '_alice'].forEach(name => {
				expect(isValidNamespaceName(name), `name: ${name}`).to.equal(false);
			});
		});

		it('returns false when any character is invalid', () => {
			['al.ce', 'alIce', 'al ce', 'al@ce', 'al#ce'].forEach(name => {
				expect(isValidNamespaceName(name), `name: ${name}`).to.equal(false);
			});
		});
	});

	// endregion
});

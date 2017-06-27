import { expect } from 'chai';
import { sha3_256 } from 'js-sha3';
import idGenerator from '../../src/model/idGenerator';

const constants = {
	nem_id: [0x375FFA4B, 0x84B3552D],
	xem_id: [0xD95FCF29, 0xD525AD41],
	namespace_base_id: [0, 0]
};

describe('id generator', () => {
	function generateId(parentId, name) {
		const hash = sha3_256.create();
		hash.update(Uint32Array.from(parentId).buffer);
		hash.update(name);
		const result = new Uint32Array(hash.arrayBuffer());
		return [result[0], result[1]];
	}

	function addBasicTests(generator) {
		it('produces different results for different names', () => {
			// Assert:
			for (const name of ['bloodyrookie.alice', 'nem.mex', 'bloodyrookie.xem', 'bloody_rookie.xem'])
				expect(generator(name), `nem.xem vs ${name}`).to.not.equal(generator('nem.xem'));
		});

		it('rejects names with uppercase characters', () => {
			// Assert:
			for (const name of ['NEM.xem', 'NEM.XEM', 'nem.XEM', 'nEm.XeM', 'NeM.xEm'])
				expect(() => generator(name), `name ${name}`).to.throw('invalid part name');
		});

		it('rejects improper qualified names', () => {
			// Assert:
			for (const name of ['.', '..', '...', '.a', 'b.', 'a..b', '.a.b', 'b.a.'])
				expect(() => generator(name), `name ${name}`).to.throw('empty part');
		});

		it('rejects improper part names', () => {
			// Assert:
			for (const name of ['alpha.bet@.zeta', 'a!pha.beta.zeta', 'alpha.beta.ze^a'])
				expect(() => generator(name), `name ${name}`).to.throw('invalid part name');
		});

		it('rejects empty string', () => {
			// Assert:
			expect(() => generator(''), 'empty string').to.throw('having zero length');
		});
	}

	describe('generate mosaic id', () => {
		it('generates correct well known id', () => {
			// Assert:
			expect(idGenerator.generateMosaicId('nem:xem')).to.deep.equal(constants.xem_id);
		});

		it('supports multi level mosaics', () => {
			// Arrange:
			const expected = generateId(
				generateId(generateId(generateId(constants.namespace_base_id, 'foo'), 'bar'), 'baz'),
				'tokens');

			// Assert:
			expect(idGenerator.generateMosaicId('foo.bar.baz:tokens')).to.deep.equal(expected);
		});

		it('rejects namespace only names', () => {
			// Assert:
			for (const name of ['bloodyrookie.alice', 'nem.mex', 'bloodyrookie.xem', 'bloody_rookie.xem'])
				expect(() => idGenerator.generateMosaicId(name), `name ${name}`).to.throw('missing mosaic');
		});

		it('rejects mosaic only names', () => {
			// Assert:
			for (const name of ['nem', 'xem', 'alpha'])
				expect(() => idGenerator.generateMosaicId(name), `name ${name}`).to.throw('missing mosaic');
		});

		it('rejects names with too many parts', () => {
			// Assert:
			for (const name of ['a.b.c.d:e', 'a.b.c.d.e:f'])
				expect(() => idGenerator.generateMosaicId(name), `name ${name}`).to.throw('too many parts');
		});

		it('rejects improper mosaic qualified names', () => {
			// Assert:
			for (const name of ['a:b:c', 'a::b'])
				expect(() => idGenerator.generateMosaicId(name), `name ${name}`).to.throw('invalid part name');
		});

		addBasicTests(namespaceName => {
			const separatorIndex = namespaceName.lastIndexOf('.');
			const namespaceAndMosaicName = 0 > separatorIndex
				? namespaceName
				: `${namespaceName.substr(0, separatorIndex)}:${namespaceName.substr(separatorIndex + 1)}`;

			// Act:
			return idGenerator.generateMosaicId(namespaceAndMosaicName);
		});
	});

	describe('generate namespace paths', () => {
		it('generates correct well known root path', () => {
			// Act:
			const path = idGenerator.generateNamespacePath('nem');

			// Assert:
			expect(path.length).to.equal(1);
			expect(path[0]).to.deep.equal(constants.nem_id);
		});

		it('generates correct well known child path', () => {
			// Act:
			const path = idGenerator.generateNamespacePath('nem.xem');

			// Assert:
			expect(path.length).to.equal(2);
			expect(path[0]).to.deep.equal(constants.nem_id);
			expect(path[1]).to.deep.equal(constants.xem_id);
		});

		it('supports multi level namespaces', () => {
			// Arrange:
			const expected = [];
			expected.push(generateId(constants.namespace_base_id, 'foo'));
			expected.push(generateId(expected[0], 'bar'));
			expected.push(generateId(expected[1], 'baz'));

			// Assert:
			expect(idGenerator.generateNamespacePath('foo.bar.baz')).to.deep.equal(expected);
		});

		it('rejects names with too many parts', () => {
			// Assert:
			for (const name of ['a.b.c.d', 'a.b.c.d.e'])
				expect(() => idGenerator.generateNamespacePath(name), `name ${name}`).to.throw('too many parts');
		});

		addBasicTests(idGenerator.generateNamespacePath);
	});
});

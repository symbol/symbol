import { expect } from 'chai';
import EntityType from '../../src/model/EntityType';
import ModelType from '../../src/model/ModelType';
import catapultModelSystem from '../../src/plugins/catapultModelSystem';

const formattingRules = {
	[ModelType.none]: () => 'none',
	[ModelType.binary]: () => 'binary',
	[ModelType.uint64]: () => 'uint64',
	[ModelType.objectId]: () => 'objectId',
	[ModelType.string]: () => 'string'
};

describe('catapult model system', () => {
	describe('basic', () => {
		it('cannot register unknown extension', () => {
			// Act:
			expect(() => catapultModelSystem.configure(['transfer', 'foo', 'namespace'])).to.throw('plugin \'foo\' not supported');
		});

		it('has support for all plugins', () => {
			// Act:
			const supportedPluginNames = catapultModelSystem.supportedPluginNames();

			// Assert:
			expect(supportedPluginNames).to.deep.equal(['aggregate', 'multisig', 'namespace', 'transfer']);
		});
	});

	describe('model schema', () => {
		it('can create default schema', () => {
			// Act:
			const system = catapultModelSystem.configure();

			// Assert:
			expect(system.schema).to.contain.key('blockHeader');
			expect(system.schema).to.not.contain.key('transfer');
			expect(system.schema).to.not.contain.key('registerNamespace');
		});

		it('can register single extension', () => {
			// Act:
			const system = catapultModelSystem.configure(['transfer'], formattingRules);

			// Assert:
			expect(system.schema).to.contain.key('blockHeader');
			expect(system.schema).to.contain.key('transfer');
			expect(system.schema).to.not.contain.key('registerNamespace');
		});

		it('can register multiple extensions', () => {
			// Act:
			const system = catapultModelSystem.configure(['transfer', 'namespace', 'aggregate'], formattingRules);

			// Assert:
			expect(system.schema).to.contain.key('blockHeader');
			expect(system.schema).to.contain.key('transfer');
			expect(system.schema).to.contain.key('registerNamespace');
			expect(system.schema).to.contain.key('aggregate');
		});
	});

	describe('codec', () => {
		it('can create default codec', () => {
			// Act:
			const system = catapultModelSystem.configure();

			// Assert:
			expect(system.codec.supports(0x8000)).to.equal(true);
			expect(system.codec.supports(EntityType.transfer)).to.equal(false);
			expect(system.codec.supports(EntityType.registerNamespace)).to.equal(false);
		});

		it('can register single extension', () => {
			// Act:
			const system = catapultModelSystem.configure(['transfer'], formattingRules);

			// Assert:
			expect(system.codec.supports(0x8000)).to.equal(true);
			expect(system.codec.supports(EntityType.transfer)).to.equal(true);
			expect(system.codec.supports(EntityType.registerNamespace)).to.equal(false);
		});

		it('can register multiple extensions', () => {
			// Act:
			const system = catapultModelSystem.configure(['transfer', 'namespace'], formattingRules);

			// Assert:
			expect(system.codec.supports(0x8000)).to.equal(true);
			expect(system.codec.supports(EntityType.transfer)).to.equal(true);
			expect(system.codec.supports(EntityType.registerNamespace)).to.equal(true);
		});
	});

	describe('model formatter', () => {
		it('can create default formatter', () => {
			// Act:
			const system = catapultModelSystem.configure();

			// Assert:
			expect(system.formatter).to.contain.key('blockHeaderWithMetadata');
			expect(system.formatter).to.not.contain.key('transfer');
			expect(system.formatter).to.not.contain.key('registerNamespace');
		});

		it('can register single extension', () => {
			// Act:
			const system = catapultModelSystem.configure(['transfer'], formattingRules);

			// Assert:
			expect(system.formatter).to.contain.key('blockHeaderWithMetadata');
			expect(system.formatter).to.contain.key('transfer');
			expect(system.formatter).to.not.contain.key('registerNamespace');
		});

		it('can register multiple extensions', () => {
			// Act:
			const system = catapultModelSystem.configure(['transfer', 'namespace'], formattingRules);

			// Assert:
			expect(system.formatter).to.contain.key('blockHeaderWithMetadata');
			expect(system.formatter).to.contain.key('transfer');
			expect(system.formatter).to.contain.key('registerNamespace');
		});
	});
});

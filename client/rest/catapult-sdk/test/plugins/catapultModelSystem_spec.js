/*
 * Copyright (c) 2016-present,
 * Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
 *
 * This file is part of Catapult.
 *
 * Catapult is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Catapult is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Catapult.  If not, see <http://www.gnu.org/licenses/>.
 */

const catapultModelSystem = require('../../src/plugins/catapultModelSystem');
const EntityType = require('../../src/model/EntityType');
const ModelType = require('../../src/model/ModelType');
const { expect } = require('chai');

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
			expect(supportedPluginNames).to.deep.equal([
				'accountLink',
				'accountProperties',
				'aggregate',
				'lock',
				'mosaic',
				'multisig',
				'namespace',
				'receipts',
				'transfer'
			]);
		});
	});

	describe('model schema', () => {
		it('can create default schema', () => {
			// Act:
			const system = catapultModelSystem.configure([], {});

			// Assert:
			expect(system.schema).to.contain.key('blockHeader');
			expect(system.schema).to.not.contain.key('transfer');
			expect(system.schema).to.not.contain.key('registerNamespace');
		});

		it('can register single extension', () => {
			// Act:
			const system = catapultModelSystem.configure(['transfer'], {});

			// Assert:
			expect(system.schema).to.contain.key('blockHeader');
			expect(system.schema).to.contain.key('transfer');
			expect(system.schema).to.not.contain.key('registerNamespace');
		});

		it('can register multiple extensions', () => {
			// Act:
			const system = catapultModelSystem.configure(['transfer', 'namespace', 'aggregate'], {});

			// Assert:
			expect(system.schema).to.contain.key('blockHeader');
			expect(system.schema).to.contain.key('transfer');
			expect(system.schema).to.contain.key('registerNamespace');
			expect(system.schema).to.contain.key('aggregateComplete');
		});
	});

	describe('codec', () => {
		it('can create default codec', () => {
			// Act:
			const system = catapultModelSystem.configure([], {});

			// Assert:
			expect(system.codec.supports(0x8000)).to.equal(true);
			expect(system.codec.supports(EntityType.transfer)).to.equal(false);
			expect(system.codec.supports(EntityType.registerNamespace)).to.equal(false);
		});

		it('can register single extension', () => {
			// Act:
			const system = catapultModelSystem.configure(['transfer'], {});

			// Assert:
			expect(system.codec.supports(0x8000)).to.equal(true);
			expect(system.codec.supports(EntityType.transfer)).to.equal(true);
			expect(system.codec.supports(EntityType.registerNamespace)).to.equal(false);
		});

		it('can register multiple extensions', () => {
			// Act:
			const system = catapultModelSystem.configure(['transfer', 'namespace'], {});

			// Assert:
			expect(system.codec.supports(0x8000)).to.equal(true);
			expect(system.codec.supports(EntityType.transfer)).to.equal(true);
			expect(system.codec.supports(EntityType.registerNamespace)).to.equal(true);
		});
	});

	describe('model formatter', () => {
		it('can create default formatter without extensions', () => {
			// Act:
			const system = catapultModelSystem.configure([], { default: formattingRules });

			// Assert:
			expect(Object.keys(system.formatters)).to.deep.equal(['default']);

			const formatter = system.formatters.default;
			expect(formatter).to.contain.key('blockHeaderWithMetadata');
			expect(formatter).to.not.contain.key('transfer');
			expect(formatter).to.not.contain.key('registerNamespace');
		});

		it('can register single extension', () => {
			// Act:
			const system = catapultModelSystem.configure(['transfer'], { default: formattingRules });

			// Assert:
			expect(Object.keys(system.formatters)).to.deep.equal(['default']);

			const formatter = system.formatters.default;
			expect(formatter).to.contain.key('blockHeaderWithMetadata');
			expect(formatter).to.contain.key('transfer');
			expect(formatter).to.not.contain.key('registerNamespace');
		});

		it('can register multiple extensions', () => {
			// Act:
			const system = catapultModelSystem.configure(['transfer', 'namespace'], { default: formattingRules });

			// Assert:
			expect(Object.keys(system.formatters)).to.deep.equal(['default']);

			const formatter = system.formatters.default;
			expect(formatter).to.contain.key('blockHeaderWithMetadata');
			expect(formatter).to.contain.key('transfer');
			expect(formatter).to.contain.key('registerNamespace');
		});

		it('cannot create formatter when no rules are specified', () => {
			// Act:
			const system = catapultModelSystem.configure(['transfer'], {});

			// Assert:
			expect(Object.keys(system.formatters)).to.deep.equal([]);
		});

		it('can create multiple formatters when multiple sets of formatting rules are specfied', () => {
			// Act:
			const system = catapultModelSystem.configure(['transfer'], {
				default: formattingRules,
				custom: { [ModelType.uint64]: value => value[0] }
			});

			// Assert:
			expect(Object.keys(system.formatters)).to.deep.equal(['default', 'custom']);

			// - plugins are respected
			['default', 'custom'].forEach(key => {
				const formatter = system.formatters[key];
				const message = `formatter ${key}`;
				expect(formatter, message).to.contain.key('blockHeaderWithMetadata');
				expect(formatter, message).to.contain.key('transfer');
				expect(formatter, message).to.not.contain.key('registerNamespace');
			});

			// - formatting rules are dependent on formatter
			const chainInfo = { height: [123, 456] };
			expect(system.formatters.default.chainInfo.format(chainInfo)).to.deep.equal({ height: 'uint64' });
			expect(system.formatters.custom.chainInfo.format(chainInfo)).to.deep.equal({ height: 123 });
		});
	});
});

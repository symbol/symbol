/*
 * Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
 * Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
 * All rights reserved.
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

const { ServerMessageHandler } = require('../../src/connection/serverMessageHandlers');
const test = require('../testUtils');
const { expect } = require('chai');

describe('server message handlers', () => {
	const createMockCodec = value => {
		const mock = {
			// only some message handlers require codec, objects passed to codec.deserialize() are collected in following array
			collected: [],
			deserialize: (parser, options) => {
				mock.collected.push({ parser, options });
				return value;
			}
		};
		return mock;
	};

	it('block handler', () => {
		// Arrange:
		const emitted = [];
		const codec = createMockCodec(34);
		const blockBuffer = Buffer.of(0xAB, 0xCD, 0xEF);

		// Act:
		ServerMessageHandler.block(codec, eventData => emitted.push(eventData))(12, blockBuffer, 56, 78, 99, 88);

		// Assert:
		// - 12 is a "topic" so it's not forwarded
		// - trailing params (99, 88) should be ignored
		expect(codec.collected.length).to.equal(1);
		expect(codec.collected[0].parser.buffers.current()).to.equal(blockBuffer);

		expect(emitted.length).to.equal(1);
		expect(emitted[0]).to.deep.equal({
			type: 'blockHeaderWithMetadata',
			payload: { block: 34, meta: { hash: 56, generationHash: 78 } }
		});
	});

	it('finalized block handler', () => {
		// Arrange:
		const emitted = [];
		const codec = createMockCodec(35);
		const finalizedBlockBuffer = Buffer.concat([
			Buffer.of(44, 0, 0, 0), // finalization epoch
			Buffer.of(55, 0, 0, 0), // finalization point
			Buffer.of(66, 0, 0, 0, 0, 0, 0, 0), // height
			Buffer.alloc(test.constants.sizes.hash256, 41) // hash
		]);

		// Act:
		ServerMessageHandler.finalizedBlock(codec, eventData => emitted.push(eventData))(12, finalizedBlockBuffer, 99, 88);

		// Assert:
		// - 12 is a "topic" so it's not forwarded
		// - trailing params (99, 88) should be ignored
		expect(codec.collected.length).to.equal(0);

		expect(emitted.length).to.equal(1);
		expect(emitted[0]).to.deep.equal({
			type: 'finalizedBlock',
			payload: {
				finalizationEpoch: 44,
				finalizationPoint: 55,
				height: [66, 0],
				hash: Buffer.alloc(test.constants.sizes.hash256, 41)
			}
		});
	});

	it('transaction handler', () => {
		// Arrange:
		const emitted = [];
		const codec = createMockCodec(33);
		const transactionBuffer = Buffer.of(0xEF, 0xCD, 0xAB);

		// Act:
		const height = Buffer.of(66, 0, 0, 0, 0, 0, 0, 0);
		ServerMessageHandler.transaction(codec, eventData => emitted.push(eventData))(22, transactionBuffer, 44, 55, height, 77, 88, 99);

		// Assert:
		// - 22 is a "topic" so it's not forwarded
		// - trailing params (77, 88, 99) should be ignored
		expect(codec.collected.length).to.equal(1);
		expect(codec.collected[0].parser.buffers.current()).to.equal(transactionBuffer);
		expect(codec.collected[0].options).to.equal(undefined);

		expect(emitted.length).to.equal(1);
		expect(emitted[0]).to.deep.equal({
			type: 'transactionWithMetadata',
			payload: {
				transaction: 33,
				meta: {
					hash: 44,
					merkleComponentHash: 55,
					height: [66, 0]
				}
			}
		});
	});

	it('transaction hash handler', () => {
		// Arrange:
		const emitted = [];
		const codec = createMockCodec(35);

		// Act:
		ServerMessageHandler.transactionHash(codec, eventData => emitted.push(eventData))(22, 44, 77, 88, 99);

		// Assert:
		// - 22 is a "topic" so it's not forwarded
		// - trailing params (77, 88, 99) should be ignored
		expect(codec.collected.length).to.equal(0);

		expect(emitted.length).to.equal(1);
		expect(emitted[0]).to.deep.equal({ type: 'transactionWithMetadata', payload: { meta: { hash: 44 } } });
	});

	it('transaction status handler', () => {
		// Arrange:
		const emitted = [];
		const codec = createMockCodec(35);
		const buffer = Buffer.concat([
			Buffer.alloc(test.constants.sizes.hash256, 41), // hash
			Buffer.of(66, 0, 0, 0, 0, 0, 0, 0), // deadline
			Buffer.of(55, 0, 0, 0) // status
		]);

		// Act:
		ServerMessageHandler.transactionStatus(codec, eventData => emitted.push(eventData))(22, buffer, 99);

		// Assert:
		// - 22 is a "topic" so it's not forwarded
		// - trailing param 99 should be ignored
		expect(codec.collected.length).to.equal(0);

		expect(emitted.length).to.equal(1);
		expect(emitted[0]).to.deep.equal({
			type: 'transactionStatus',
			payload: {
				hash: Buffer.alloc(test.constants.sizes.hash256, 41),
				code: 55,
				deadline: [66, 0]
			}
		});
	});
});

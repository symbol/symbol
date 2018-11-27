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

const MessageChannelBuilder = require('../../src/connection/MessageChannelBuilder');
const test = require('../testUtils');
const { expect } = require('chai');

describe('message channel builder', () => {
	const addressTemplate = {
		encoded: 'NAR3W7B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ6HDFG',
		decoded: Buffer.from('6823BB7C3C089D996585466380EDBDC19D4959184893E38CA6', 'hex')
	};

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

	const addAddressFilterTests = (markerByte, createFilter) => {
		it('rejects marker without topic param', () => {
			// Arrange:
			const filter = createFilter(new MessageChannelBuilder());

			// Act:
			expect(() => filter('')).to.throw('address param missing from address subscription');
		});

		it('accepts marker without topic param with allowOptionalAddress', () => {
			// Arrange:
			const filter = createFilter(new MessageChannelBuilder({ allowOptionalAddress: true }));

			// Act:
			const topic = filter('');

			// Assert:
			expect(topic.length).to.equal(1);
			expect(topic[0]).to.equal(markerByte);
		});

		it('accepts marker with topic param', () => {
			// Arrange:
			const filter = createFilter(new MessageChannelBuilder());

			// Act:
			const topic = filter(addressTemplate.encoded);

			// Assert:
			expect(topic.length).to.equal(test.constants.sizes.addressDecoded + 1);
			expect(topic[0]).to.equal(markerByte);
			expect(topic.slice(1)).to.deep.equal(addressTemplate.decoded);
		});

		it('rejects marker with invalid topic param', () => {
			// Arrange:
			const filter = createFilter(new MessageChannelBuilder());

			// Act:
			expect(() => filter('NAAAA')).to.throw('NAAAA does not represent a valid encoded address');
		});
	};

	const wrapHandlerEmitTest = action => {
		it('forwards to emit callback', action);
	};

	const assertTransactionHandlerEmit = (channelName, createHandler) => {
		// Arrange:
		const emitted = [];
		const codec = createMockCodec(33);
		const handler = createHandler(new MessageChannelBuilder());
		const transactionBuffer = Buffer.of(0xEF, 0xCD, 0xAB);

		// Act:
		const height = Buffer.of(66, 0, 0, 0, 0, 0, 0, 0);
		handler(codec, eventData => emitted.push(eventData))(22, transactionBuffer, 44, 55, height, 77, 88, 99);

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
					height: [66, 0],
					channelName
				}
			}
		});
	};

	const assertTransactionHashHandlerEmit = (channelName, createHandler) => {
		// Arrange:
		const emitted = [];
		const codec = createMockCodec(35);
		const handler = createHandler(new MessageChannelBuilder());

		// Act:
		handler(codec, eventData => emitted.push(eventData))(22, 44, 77, 88, 99);

		// Assert:
		// - 22 is a "topic" so it's not forwarded
		// - trailing params (77, 88, 99) should be ignored
		expect(codec.collected.length).to.equal(0);

		expect(emitted.length).to.equal(1);
		expect(emitted[0]).to.deep.equal({ type: 'transactionWithMetadata', payload: { meta: { hash: 44, channelName } } });
	};

	describe('default channels', () => {
		it('are all present', () => {
			// Act:
			const channels = new MessageChannelBuilder().build();
			const defaultChannelNames = Object.keys(channels);

			// Assert:
			expect(defaultChannelNames).to.deep.equal(['block', 'confirmedAdded', 'unconfirmedAdded', 'unconfirmedRemoved', 'status']);
		});

		describe('block', () => {
			describe('filter', () => {
				it('accepts marker without topic param', () => {
					// Arrange:
					const { filter } = new MessageChannelBuilder().build().block;

					// Act:
					const topic = filter('');

					// Assert:
					expect(topic).to.deep.equal(Buffer.of(0x49, 0x6A, 0xCA, 0x80, 0xE4, 0xD8, 0xF2, 0x9F));
				});

				it('rejects marker with topic param', () => {
					// Arrange:
					const { filter } = new MessageChannelBuilder().build().block;

					// Act:
					expect(() => filter(addressTemplate.encoded)).to.throw('unexpected param to block subscription');
				});
			});

			describe('handler', () => {
				wrapHandlerEmitTest(() => {
					// Arrange:
					const emitted = [];
					const codec = createMockCodec(34);
					const { handler } = new MessageChannelBuilder().build().block;
					const blockBuffer = Buffer.of(0xAB, 0xCD, 0xEF);

					// Act:
					handler(codec, eventData => emitted.push(eventData))(12, blockBuffer, 56, 78, 99, 88);

					// Assert:
					// - 12 is a "topic" so it's not forwarded
					// - trailing params (99, 88) should be ignored
					expect(codec.collected.length).to.equal(1);
					expect(codec.collected[0].parser.buffers.current()).to.equal(blockBuffer);
					expect(codec.collected[0].options).to.deep.equal({ skipBlockTransactions: true });

					expect(emitted.length).to.equal(1);
					expect(emitted[0]).to.deep.equal({
						type: 'blockHeaderWithMetadata',
						payload: { block: 34, meta: { hash: 56, generationHash: 78 } }
					});
				});
			});
		});

		describe('confirmedAdded', () => {
			describe('filter', () => { addAddressFilterTests(0x61, builder => builder.build().confirmedAdded.filter); });
			describe('handler', () => {
				wrapHandlerEmitTest(() =>
					assertTransactionHandlerEmit('confirmedAdded', builder => builder.build().confirmedAdded.handler));
			});
		});

		describe('unconfirmedAdded', () => {
			describe('filter', () => { addAddressFilterTests(0x75, builder => builder.build().unconfirmedAdded.filter); });
			describe('handler', () => {
				wrapHandlerEmitTest(() =>
					assertTransactionHandlerEmit('unconfirmedAdded', builder => builder.build().unconfirmedAdded.handler));
			});
		});

		describe('unconfirmedRemoved', () => {
			describe('filter', () => { addAddressFilterTests(0x72, builder => builder.build().unconfirmedRemoved.filter); });
			describe('handler', () => {
				wrapHandlerEmitTest(() =>
					assertTransactionHashHandlerEmit('unconfirmedRemoved', builder => builder.build().unconfirmedRemoved.handler));
			});
		});

		describe('status', () => {
			describe('filter', () => { addAddressFilterTests(0x73, builder => builder.build().status.filter); });
			describe('handler', () => {
				wrapHandlerEmitTest(() => {
					// Arrange:
					const emitted = [];
					const codec = createMockCodec(35);
					const { handler } = new MessageChannelBuilder().build().status;

					// Act:
					const buffer = Buffer.concat([
						Buffer.alloc(test.constants.sizes.hash256, 41), // hash
						Buffer.of(55, 0, 0, 0), // status
						Buffer.of(66, 0, 0, 0, 0, 0, 0, 0) // deadline
					]);
					handler(codec, eventData => emitted.push(eventData))(22, buffer, 99);

					// Assert:
					// - 22 is a "topic" so it's not forwarded
					// - trailing param 99 should be ignored
					expect(codec.collected.length).to.equal(0);

					expect(emitted.length).to.equal(1);
					expect(emitted[0]).to.deep.equal({
						type: 'transactionStatus',
						payload: {
							hash: Buffer.alloc(test.constants.sizes.hash256, 41),
							status: 55,
							deadline: [66, 0]
						}
					});
				});
			});
		});
	});

	describe('custom channels', () => {
		describe('with transaction handler', () => {
			const createChannelInfo = builder => {
				builder.add('foo', 'z', 'transaction');
				return builder.build().foo;
			};
			describe('filter', () => { addAddressFilterTests(0x7A, builder => createChannelInfo(builder).filter); });
			describe('handler', () => {
				wrapHandlerEmitTest(() => assertTransactionHandlerEmit('foo', builder => createChannelInfo(builder).handler));
			});
		});

		describe('with transaction hash handler', () => {
			const createChannelInfo = builder => {
				builder.add('foo', 'z', 'transactionHash');
				return builder.build().foo;
			};
			describe('filter', () => { addAddressFilterTests(0x7A, builder => createChannelInfo(builder).filter); });
			describe('handler', () => {
				wrapHandlerEmitTest(() => assertTransactionHashHandlerEmit('foo', builder => createChannelInfo(builder).handler));
			});
		});

		describe('with custom handler', () => {
			const createChannelInfo = builder => {
				builder.add('foo', 'z', (codec, emit) => (topic, buffer) => {
					// just emit first byte
					emit({ first: buffer[0] });
				});
				return builder.build().foo;
			};
			describe('filter', () => { addAddressFilterTests(0x7A, builder => createChannelInfo(builder).filter); });
			describe('handler', () => {
				wrapHandlerEmitTest(() => {
					// Arrange:
					const emitted = [];
					const codec = createMockCodec(40);
					const { handler } = createChannelInfo(new MessageChannelBuilder());

					// Act:
					const buffer = [55, 77, 33];
					handler(codec, eventData => emitted.push(eventData))(22, buffer, 99);

					// Assert:
					// - 22 is a "topic" so it's not forwarded
					// - trailing param 99 should be ignored
					expect(codec.collected.length).to.equal(0);

					expect(emitted.length).to.equal(1);
					expect(emitted[0]).to.deep.equal({ first: 55 });
				});
			});
		});

		it('cannot be added with unknown handler', () => {
			// Arrange:
			const builder = new MessageChannelBuilder();

			// Assert:
			expect(() => builder.add('foo', 'z', 'status')).to.throw('unknown handler');
		});

		it('cannot be added with multi-character marker', () => {
			// Arrange:
			const builder = new MessageChannelBuilder();

			// Assert:
			expect(() => builder.add('foo', 'zz', 'transaction')).to.throw('channel marker must be single character');
		});

		it('cannot override default channel', () => {
			// Arrange:
			const builder = new MessageChannelBuilder();

			// Assert:
			expect(() => builder.add('status', 'z', 'transaction')).to.throw('channel has already been registered');
			expect(() => builder.add('foo', 'u', 'transaction')).to.throw('channel marker has already been registered');
		});

		it('cannot be registered multiple times', () => {
			// Arrange:
			const builder = new MessageChannelBuilder();
			builder.add('foo', 'z', 'transaction');

			// Assert:
			expect(() => builder.add('foo', 'z', 'transaction')).to.throw('channel has already been registered');
			expect(() => builder.add('foo', 'y', 'transaction')).to.throw('channel has already been registered');
			expect(() => builder.add('bar', 'z', 'transaction')).to.throw('channel marker has already been registered');
		});
	});
});

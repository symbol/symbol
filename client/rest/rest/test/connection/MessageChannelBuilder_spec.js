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

const MessageChannelBuilder = require('../../src/connection/MessageChannelBuilder');
const { ServerMessageHandler } = require('../../src/connection/serverMessageHandlers');
const test = require('../testUtils');
const { expect } = require('chai');

describe('message channel builder', () => {
	const addressTemplate = {
		encoded: 'NAR3W7B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ6HDA',
		decoded: Buffer.from('6823BB7C3C089D996585466380EDBDC19D4959184893E38C', 'hex')
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

	describe('default channels', () => {
		it('are all present', () => {
			// Act:
			const channels = new MessageChannelBuilder().build();
			const defaultChannelNames = Object.keys(channels);

			// Assert:
			expect(defaultChannelNames).to.deep.equal([
				'block',
				'finalizedBlock',
				'confirmedAdded',
				'unconfirmedAdded',
				'unconfirmedRemoved',
				'status'
			]);
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
				it('is set to block type', () => {
					const messageChannelBuilder = new MessageChannelBuilder();
					expect(messageChannelBuilder.descriptors.block.handler).to.equal(ServerMessageHandler.block);
				});
			});
		});

		describe('confirmedAdded', () => {
			describe('filter', () => { addAddressFilterTests(0x61, builder => builder.build().confirmedAdded.filter); });
			describe('handler', () => {
				it('is set to transaction type', () => {
					const messageChannelBuilder = new MessageChannelBuilder();
					expect(messageChannelBuilder.descriptors.confirmedAdded.handler).to.equal(ServerMessageHandler.transaction);
				});
			});
		});

		describe('unconfirmedAdded', () => {
			describe('filter', () => { addAddressFilterTests(0x75, builder => builder.build().unconfirmedAdded.filter); });
			describe('handler', () => {
				it('is set to transaction type', () => {
					const messageChannelBuilder = new MessageChannelBuilder();
					expect(messageChannelBuilder.descriptors.unconfirmedAdded.handler).to.equal(ServerMessageHandler.transaction);
				});
			});
		});

		describe('unconfirmedRemoved', () => {
			describe('filter', () => { addAddressFilterTests(0x72, builder => builder.build().unconfirmedRemoved.filter); });
			describe('handler', () => {
				it('is set to transaction type', () => {
					const messageChannelBuilder = new MessageChannelBuilder();
					expect(messageChannelBuilder.descriptors.unconfirmedRemoved.handler).to.equal(ServerMessageHandler.transactionHash);
				});
			});
		});

		describe('status', () => {
			describe('filter', () => { addAddressFilterTests(0x73, builder => builder.build().status.filter); });
			describe('handler', () => {
				it('is set to transaction type', () => {
					const messageChannelBuilder = new MessageChannelBuilder();
					expect(messageChannelBuilder.descriptors.status.handler).to.equal(ServerMessageHandler.transactionStatus);
				});
			});
		});
	});

	describe('custom channels', () => {
		describe('with transaction handler', () => {
			const createChannelInfo = builder => {
				builder.add('foo', 'z', ServerMessageHandler.transaction);
				return builder.build().foo;
			};
			describe('filter', () => { addAddressFilterTests(0x7A, builder => createChannelInfo(builder).filter); });
			describe('handler', () => {
				it('is set to transaction type', () => {
					const messageChannelBuilder = new MessageChannelBuilder();
					createChannelInfo(messageChannelBuilder);
					expect(messageChannelBuilder.descriptors.foo.handler).to.equal(ServerMessageHandler.transaction);
				});
			});
		});

		describe('with transaction hash handler', () => {
			const createChannelInfo = builder => {
				builder.add('foo', 'z', ServerMessageHandler.transactionHash);
				return builder.build().foo;
			};
			describe('filter', () => { addAddressFilterTests(0x7A, builder => createChannelInfo(builder).filter); });
			describe('handler', () => {
				it('is set to transaction hash type', () => {
					const messageChannelBuilder = new MessageChannelBuilder();
					createChannelInfo(messageChannelBuilder);
					expect(messageChannelBuilder.descriptors.foo.handler).to.equal(ServerMessageHandler.transactionHash);
				});
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
				it('forwards to emit callback', () => {
					// Arrange:
					const emitted = [];
					const codec = {
						collected: [],
						deserialize: (parser, options) => {
							codec.collected.push({ parser, options });
							return 40;
						}
					};
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

		it('cannot be added with multi-character marker', () => {
			// Arrange:
			const builder = new MessageChannelBuilder();

			// Assert:
			expect(() => builder.add('foo', 'zz', ServerMessageHandler.transaction)).to.throw('channel marker must be single character');
		});

		it('cannot override default channel', () => {
			// Arrange:
			const builder = new MessageChannelBuilder();

			// Assert:
			expect(() => builder.add('status', 'z', ServerMessageHandler.transaction)).to.throw('channel has already been registered');
			expect(() => builder.add('foo', 'u', ServerMessageHandler.transaction)).to.throw('channel marker has already been registered');
		});

		it('cannot be registered multiple times', () => {
			// Arrange:
			const builder = new MessageChannelBuilder();
			builder.add('foo', 'z', ServerMessageHandler.transaction);

			// Assert:
			expect(() => builder.add('foo', 'z', ServerMessageHandler.transaction)).to.throw('channel has already been registered');
			expect(() => builder.add('foo', 'y', ServerMessageHandler.transaction)).to.throw('channel has already been registered');
			expect(() => builder.add('bar', 'z', ServerMessageHandler.transaction)).to.throw('channel marker has already been registered');
		});
	});
});

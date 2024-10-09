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

import ServerMessageHandler from '../../src/connection/serverMessageHandlers.js';
import test from '../testUtils.js';
import { expect } from 'chai';
import { Hash256, utils } from 'symbol-sdk';
import { models } from 'symbol-sdk/symbol';

describe('server message handlers', () => {
	// region block handler

	describe('block handler', () => {
		const runBlockHandlerTest = (blockBuffer, expectedBlock) => {
			// Arrange:
			const emitted = [];

			// Act:
			ServerMessageHandler.block(eventData => emitted.push(eventData))(12, blockBuffer, 56, 78, 99, 88);

			// Assert:
			// - 12 is a "topic" so it's not forwarded
			// - trailing params (99, 88) should be ignored
			expect(emitted.length).to.equal(1);
			expect(emitted[0]).to.deep.equal({
				type: 'blockHeaderWithMetadata',
				payload: { block: expectedBlock, meta: { hash: 56, generationHash: 78 } }
			});
		};

		it('can handle block header (without transactions) ', () => {
			// Arrange:
			const sampleBlock = test.createSampleBlock();

			// Act + Assert:
			runBlockHandlerTest(sampleBlock.buffer, sampleBlock.model);
		});

		it('can handle block header (with transactions) ', () => {
			// Arrange:
			const sampleBlock = test.createSampleBlock();

			// Act + Assert: change size to be (much) larger than block header and available data to emulate a block with transactions
			runBlockHandlerTest(
				new Uint8Array([
					...utils.hexToUint8('99887766'),
					...sampleBlock.buffer.subarray(4)
				]),
				sampleBlock.model
			);
		});
	});

	// endregion

	// region finalized block handler

	it('finalized block handler', () => {
		// Arrange:
		const emitted = [];
		const finalizedBlockBuffer = Buffer.concat([
			Buffer.of(44, 0, 0, 0), // finalization epoch
			Buffer.of(55, 0, 0, 0), // finalization point
			Buffer.of(66, 0, 0, 0, 0, 0, 0, 0), // height
			Buffer.alloc(Hash256.SIZE, 41) // hash
		]);

		// Act:
		ServerMessageHandler.finalizedBlock(eventData => emitted.push(eventData))(12, finalizedBlockBuffer, 99, 88);

		// Assert:
		// - 12 is a "topic" so it's not forwarded
		// - trailing params (99, 88) should be ignored
		expect(emitted.length).to.equal(1);
		expect(emitted[0]).to.deep.equal({
			type: 'finalizedBlock',
			payload: {
				finalizationEpoch: 44,
				finalizationPoint: 55,
				height: 66n,
				hash: Buffer.alloc(Hash256.SIZE, 41)
			}
		});
	});

	// endregion

	// region transaction handler

	describe('transaction handler', () => {
		const runTransactionHandlerTest = (transactionBuffer, assertEmitted) => {
			// Arrange:
			const emitted = [];

			// Act:
			const height = Buffer.of(66, 0, 0, 0, 0, 0, 0, 0);
			ServerMessageHandler.transaction(eventData => emitted.push(eventData))(22, transactionBuffer, 44, 55, height, 77, 88, 99);

			// Assert:
			// - 22 is a "topic" so it's not forwarded
			// - trailing params (77, 88, 99) should be ignored
			expect(emitted.length).to.equal(1);
			assertEmitted(emitted[0]);
		};

		it('can handle simple transaction', () => {
			// Arrange:
			const sampleTransaction = test.createSampleTransaction();

			// Act:
			runTransactionHandlerTest(sampleTransaction.buffer, emitted => {
				// Assert:
				expect(emitted).to.deep.equal({
					type: 'transactionWithMetadata',
					payload: {
						transaction: sampleTransaction.model,
						meta: {
							hash: 44,
							merkleComponentHash: 55,
							height: 66n
						}
					}
				});
			});
		});

		it('can fixup transfer transaction', () => {
			// Arrange: payload is from test vector TransferTransactionV1_transfer_single_2
			const transactionBuffer = utils.hexToUint8([
				'C00000000000000042D030CD0166DA62C1DF1FF0945752475FBD2B4B975E9991EFF5',
				'7BCD742C235787433B8AF428C3852009C8C63B632572057945118F393F4187FF51DF',
				'D77CAC6DB0A48186B2D8C168DBAF2395AD3BF421F9E44D7AD8E616C5E981ABD1DB51',
				'90F20000000001985441E0FEEEEFFEEEEFFEE0711EE7711EE771989059321905F681',
				'BCF47EA33BBF5E6F8298B5440854FDED000002000000000064000000000000000200',
				'000000000000C8000000000000000100000000000000'
			].join(''));

			// Act:
			runTransactionHandlerTest(transactionBuffer, emitted => {
				// Assert: mosaicId is mapped to id
				expect(emitted.payload.transaction.mosaics).to.deep.equal([
					{ id: '100', amount: '2' },
					{ id: '200', amount: '1' }
				]);
			});
		});

		it('can fixup mosaic supply revocation transaction', () => {
			// Arrange: payload is from test vector MosaicSupplyRevocationTransactionV1_mosaic_supply_revocation_single_1
			const transactionBuffer = utils.hexToUint8([
				'A800000000000000739DFD717943C10142EEA1572C6740602E490FC2AD89850FDB9F',
				'37F5DCE16517CA34C3DE8D0E98381234D65BF4A9AF7553C22DD482B1B1CF64AFB9E4',
				'05A8768419B36A7128CEC19A49A5F6CFA50CFBC93C726132F7579A5672D9B81B4712',
				'D6E70000000001984D43E0FEEEEFFEEEEFFEE0711EE7711EE771989059321905F681',
				'BCF47EA33BBF5E6F8298B5440854FDED672B0000CE5600006400000000000000'
			].join(''));

			// Act:
			runTransactionHandlerTest(transactionBuffer, emitted => {
				// Assert: mosaic is inlined
				expect(emitted.payload.transaction).to.deep.equal({
					signature: '739DFD717943C10142EEA1572C6740602E490FC2AD89850FDB9F37F5DCE16517'
						+ 'CA34C3DE8D0E98381234D65BF4A9AF7553C22DD482B1B1CF64AFB9E405A87684',
					signerPublicKey: '19B36A7128CEC19A49A5F6CFA50CFBC93C726132F7579A5672D9B81B4712D6E7',
					version: 1,
					network: 152,
					type: 17229,
					fee: '18370164183782063840',
					deadline: '8207562320463688160',
					sourceAddress: '989059321905F681BCF47EA33BBF5E6F8298B5440854FDED',
					mosaicId: '95442763262823',
					amount: '100'
				});
			});
		});

		it('can fixup aggregate transaction', () => {
			// Arrange: payload is from test vector AggregateBondedTransactionV2_aggregate_bonded_aggregate_2
			const transactionBuffer = utils.hexToUint8([
				'20020000000000003E080DCE5B32319CA6899808CA664C3961C77A85BB42B192',
				'F36394D7B46C79FE4EC2AD6DA50E38836D4ADCDD992C020137F047C1228C351B',
				'9533176AB18CE0AFFDDDB26B9750C36F0C0F06914658E6DC86AE0C323ADBB352',
				'0D42DD85138616EB0000000002984142E0FEEEEFFEEEEFFEE0711EE7711EE771',
				'3F2BE873F569828C88CD0DE37BB31C998FA0AAEB3308A1FFBF3D01CE49E8E9F7',
				'A800000000000000600000000000000000000000000000000000000000000000',
				'0000000000000000000000000000000000000000019854419841E5B8E40781CF',
				'74DABF592817DE48711D778648DEAFB20000010000000000672B0000CE560000',
				'6400000000000000410000000000000000000000000000000000000000000000',
				'000000000000000000000000000000000000000001984D428869746E9B1A7057',
				'0A0000000000000001000000000000000000000000000000BD6072E843DF0526',
				'81FE12FCB825CC873C670BEC51E73F5B460043677D6B1EBB119DB71F2916E710',
				'BC2195251D422AF0CB2CB378C2F0F2521907F8102912EA38AD3BED2820F6AEA6',
				'656B0D89E5BDA7B2533409864B8A6C961DCA9D173AE399790000000000000000',
				'062F6371FD45C2ADB840D85B3E7AFCB22678365733264291705210A1661C6DC8',
				'F55D9667E12F30C7CEC0280A51F09F02C26F28E435E1CA1617765FB792C3AAA3',
				'350BC8ECD2116B8BDD3FC26E779C2A05DD788F0E59502E92C92DADA6C25C6A90'
			].join(''));

			// Act:
			runTransactionHandlerTest(transactionBuffer, emitted => {
				// Assert: sub-transactions are mapped to sub-objects
				const subTransactions = emitted.payload.transaction.transactions;
				expect(subTransactions.length).to.equal(2);
				expect(subTransactions.map(subTransactionWithMeta => subTransactionWithMeta.transaction.type)).to.deep.equal([
					models.TransactionType.TRANSFER.value,
					models.TransactionType.MOSAIC_SUPPLY_CHANGE.value
				]);

				// - sub transactions are fixed up
				expect(subTransactions[0].transaction.mosaics).to.deep.equal([{ id: '95442763262823', amount: '100' }]);
			});
		});
	});

	// endregion

	// region transaction hash handler

	it('transaction hash handler', () => {
		// Arrange:
		const emitted = [];

		// Act:
		ServerMessageHandler.transactionHash(eventData => emitted.push(eventData))(22, 44, 77, 88, 99);

		// Assert:
		// - 22 is a "topic" so it's not forwarded
		// - trailing params (77, 88, 99) should be ignored
		expect(emitted.length).to.equal(1);
		expect(emitted[0]).to.deep.equal({ type: 'transactionWithMetadata', payload: { meta: { hash: 44 } } });
	});

	// endregion

	// region transaction status handler

	it('transaction status handler', () => {
		// Arrange:
		const emitted = [];
		const buffer = Buffer.concat([
			Buffer.alloc(Hash256.SIZE, 41), // hash
			Buffer.of(66, 0, 0, 0, 0, 0, 0, 0), // deadline
			Buffer.of(55, 0, 0, 0) // status
		]);

		// Act:
		ServerMessageHandler.transactionStatus(eventData => emitted.push(eventData))(22, buffer, 99);

		// Assert:
		// - 22 is a "topic" so it's not forwarded
		// - trailing param 99 should be ignored
		expect(emitted.length).to.equal(1);
		expect(emitted[0]).to.deep.equal({
			type: 'transactionStatus',
			payload: {
				hash: Buffer.alloc(Hash256.SIZE, 41),
				code: 55,
				deadline: 66n
			}
		});
	});

	// endregion
});

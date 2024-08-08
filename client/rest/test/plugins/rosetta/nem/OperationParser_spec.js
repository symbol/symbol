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

import { OperationParser, convertTransactionSdkJsonToRestJson } from '../../../../src/plugins/rosetta/nem/OperationParser.js';
import { mosaicIdToString } from '../../../../src/plugins/rosetta/nem/rosettaUtils.js';
import Currency from '../../../../src/plugins/rosetta/openApi/model/Currency.js';
import Transaction from '../../../../src/plugins/rosetta/openApi/model/Transaction.js';
import TransactionIdentifier from '../../../../src/plugins/rosetta/openApi/model/TransactionIdentifier.js';
import { RosettaOperationFactory } from '../utils/rosettaTestUtils.js';
import { expect } from 'chai';
import { utils } from 'symbol-sdk';
import { NemFacade, models } from 'symbol-sdk/nem';

describe('NEM OperationParser', () => {
	// region test accounts

	// address => public key mapping
	// TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW => 9822CF9571A5551EC19720B87A567A20797B75EC4B6711387643FC352FEF704E
	// TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV => CB3F9F63C3F22DDA6D9BCD367476927426192A0497BC8AE38455D19ECB10DCC9
	// TALICE6KJ2SRSIJFVVFFH6ICUIYZ2ZZGNFUDJGRT => 727643FB1D18214334C11280DF986A0AFEE128FC1F8BE7F9118E89C417E07771
	// TDONALICE7O3L63AS3KNDCPT7ZA7HMQTFZGYUCAH => 45880194FAD01FCB55887B73EEFFDC263914ED5749BF2F3ACB928C843C57BD9A
	// TBALNEMNEMKIMWLF65HTUWMQVX5G55EBBIWS4WQC => E5F290755F021258ACE3CB29452BF38B322D76F62CAF6E9D2A89B48ABF7DD778
	// TBGJAGUAQY47BULYL4GRYBJLOI6XKXPJUXU25JRJ => 5D2EC9959153F54E5225EBBC6A677AF37DCB8C3968558F366B2841F9DA9CA14F
	// TAOPATMADWFEPME6GHOJL477SI7D3UT6NFJN4LGB => 4EEE569F2E0838489EAA0D55219D5738916D33B1379EF053920AD26548A2603E
	// TBMKRYST2J3GEZRWHS3MICWFIBSKVHH7F5FA6FH3 => 88D0C34AEA2CB96E226379E71BA6264F4460C27D29F79E24248318397AA48380

	// endregion

	// region utils

	const {
		createCosignOperation, createMultisigOperation, createTransferOperation, setOperationStatus
	} = RosettaOperationFactory;

	const lookupCurrencyDefault = mosaicId => {
		if ('currencyMosaicId' === mosaicId)
			return { currency: new Currency('currency.fee', 2) };

		if ('levy' === mosaicId.namespaceId) {
			return {
				currency: new Currency(mosaicIdToString(mosaicId), 3),
				levy: {
					currency: new Currency('levy.tax', 2),
					isAbsolute: 'absolute' === mosaicId.name,
					fee: 10,
					recipientAddress: 'TDONALICE7O3L63AS3KNDCPT7ZA7HMQTFZGYUCAH'
				}
			};
		}

		return { currency: new Currency(mosaicIdToString(mosaicId), 'nem' === mosaicId.namespaceId ? 6 : 3) };
	};

	const createDefaultParser = (network, additionalOptions = {}) => new OperationParser(network, {
		lookupCurrency: lookupCurrencyDefault,
		...additionalOptions
	});

	const parseTransaction = (parser, transaction) =>
		parser.parseTransaction(convertTransactionSdkJsonToRestJson(transaction.toJson()));

	// endregion

	describe('transaction', () => {
		// region transfer

		describe('transfer', () => {
			const assertCanParseWithSingleMosaic = async type => {
				// Arrange:
				const facade = new NemFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type,
					signerPublicKey: '9822CF9571A5551EC19720B87A567A20797B75EC4B6711387643FC352FEF704E',
					recipientAddress: 'TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV',
					amount: 12345_000000
				});

				const parser = createDefaultParser(facade.network);

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, transaction);

				// Assert:
				expect(operations).to.deep.equal([
					createTransferOperation(0, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '-12345000000', 'currency.fee', 2),
					createTransferOperation(1, 'TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV', '12345000000', 'currency.fee', 2)
				]);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal(['TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW']);
			};

			it('can parse with single mosaic (v1)', () => assertCanParseWithSingleMosaic('transfer_transaction_v1'));

			it('can parse with single mosaic (v2)', () => assertCanParseWithSingleMosaic('transfer_transaction_v2'));

			it('can parse with single mosaic in bag', async () => {
				// Arrange:
				const textEncoder = new TextEncoder();
				const facade = new NemFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type: 'transfer_transaction_v2',
					signerPublicKey: '9822CF9571A5551EC19720B87A567A20797B75EC4B6711387643FC352FEF704E',
					recipientAddress: 'TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV',
					amount: 2_000000,
					mosaics: [
						{
							mosaic: {
								mosaicId: { namespaceId: { name: textEncoder.encode('nem') }, name: textEncoder.encode('xem') },
								amount: 12345_000000
							}
						}
					]
				});

				const parser = createDefaultParser(facade.network);

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, transaction);

				// Assert:
				expect(operations).to.deep.equal([
					createTransferOperation(0, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '-24690000000', 'nem.xem', 6),
					createTransferOperation(1, 'TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV', '24690000000', 'nem.xem', 6)
				]);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal(['TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW']);
			});

			const assertCanParseWithLevy = async (levyName, levyAmount) => {
				// Arrange:
				const textEncoder = new TextEncoder();
				const facade = new NemFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type: 'transfer_transaction_v2',
					signerPublicKey: '9822CF9571A5551EC19720B87A567A20797B75EC4B6711387643FC352FEF704E',
					recipientAddress: 'TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV',
					amount: 2_000000,
					mosaics: [
						{
							mosaic: {
								mosaicId: { namespaceId: { name: textEncoder.encode('levy') }, name: textEncoder.encode(levyName) },
								amount: 12345_000000
							}
						}
					]
				});

				const parser = createDefaultParser(facade.network);

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, transaction);

				// Assert:
				expect(operations).to.deep.equal([
					createTransferOperation(0, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '-24690000000', `levy.${levyName}`, 3),
					createTransferOperation(1, 'TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV', '24690000000', `levy.${levyName}`, 3),
					createTransferOperation(2, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', `-${levyAmount}`, 'levy.tax', 2),
					createTransferOperation(3, 'TDONALICE7O3L63AS3KNDCPT7ZA7HMQTFZGYUCAH', levyAmount, 'levy.tax', 2)
				]);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal(['TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW']);
			};

			it('can parse with single mosaic in bag with absolute levy', () => assertCanParseWithLevy('absolute', '10'));

			it('can parse with single mosaic in bag with relative levy', () => assertCanParseWithLevy('relative', '24690000'));

			const createMultipleMosaicDefinition = xemAmount => {
				const textEncoder = new TextEncoder();
				return [
					{
						mosaic: {
							mosaicId: { namespaceId: { name: textEncoder.encode('baz') }, name: textEncoder.encode('baz') },
							amount: 11111_000000
						}
					},
					{
						mosaic: {
							mosaicId: { namespaceId: { name: textEncoder.encode('nem') }, name: textEncoder.encode('xem') },
							amount: xemAmount
						}
					},
					{
						mosaic: {
							mosaicId: { namespaceId: { name: textEncoder.encode('foo') }, name: textEncoder.encode('bar') },
							amount: 224433_000000
						}
					}
				];
			};

			it('can parse with multiple mosaic', async () => {
				// Arrange:
				const facade = new NemFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type: 'transfer_transaction_v2',
					signerPublicKey: '9822CF9571A5551EC19720B87A567A20797B75EC4B6711387643FC352FEF704E',
					recipientAddress: 'TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV',
					amount: 2_000000,
					mosaics: createMultipleMosaicDefinition(12345_000000)
				});

				const parser = createDefaultParser(facade.network);

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, transaction);

				// Assert:
				expect(operations).to.deep.equal([
					createTransferOperation(0, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '-22222000000', 'baz.baz', 3),
					createTransferOperation(1, 'TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV', '22222000000', 'baz.baz', 3),
					createTransferOperation(2, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '-24690000000', 'nem.xem', 6),
					createTransferOperation(3, 'TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV', '24690000000', 'nem.xem', 6),
					createTransferOperation(4, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '-448866000000', 'foo.bar', 3),
					createTransferOperation(5, 'TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV', '448866000000', 'foo.bar', 3)
				]);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal(['TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW']);
			});

			it('filters out zero transfers', async () => {
				// Arrange:
				const facade = new NemFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type: 'transfer_transaction_v2',
					signerPublicKey: '9822CF9571A5551EC19720B87A567A20797B75EC4B6711387643FC352FEF704E',
					recipientAddress: 'TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV',
					amount: 2_000000,
					mosaics: createMultipleMosaicDefinition(0)
				});

				const parser = createDefaultParser(facade.network);

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, transaction);

				// Assert:
				expect(operations).to.deep.equal([
					createTransferOperation(0, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '-22222000000', 'baz.baz', 3),
					createTransferOperation(1, 'TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV', '22222000000', 'baz.baz', 3),
					createTransferOperation(2, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '-448866000000', 'foo.bar', 3),
					createTransferOperation(3, 'TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV', '448866000000', 'foo.bar', 3)
				]);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal(['TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW']);
			});
		});

		// endregion

		// region multisig

		describe('multisig', () => {
			const createFourModifications = (modificationType1, modificationType2, modificationType3, modificationType4) => ([
				{
					modification: {
						modificationType: modificationType1,
						cosignatoryPublicKey: 'CB3F9F63C3F22DDA6D9BCD367476927426192A0497BC8AE38455D19ECB10DCC9'
					}
				},
				{
					modification: {
						modificationType: modificationType2,
						cosignatoryPublicKey: '727643FB1D18214334C11280DF986A0AFEE128FC1F8BE7F9118E89C417E07771'
					}
				},
				{
					modification: {
						modificationType: modificationType3,
						cosignatoryPublicKey: '45880194FAD01FCB55887B73EEFFDC263914ED5749BF2F3ACB928C843C57BD9A'
					}
				},
				{
					modification: {
						modificationType: modificationType4,
						cosignatoryPublicKey: 'E5F290755F021258ACE3CB29452BF38B322D76F62CAF6E9D2A89B48ABF7DD778'
					}
				}
			]);

			const assertCanParse = async options => {
				// Arrange:
				const facade = new NemFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type: `multisig_account_modification_transaction_${options.version}`,
					signerPublicKey: '9822CF9571A5551EC19720B87A567A20797B75EC4B6711387643FC352FEF704E',
					modifications: createFourModifications(...options.modificationTypes),
					...options.transactionMetadata
				});

				const parser = createDefaultParser(facade.network);

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, transaction);

				// Assert:
				expect(operations).to.deep.equal([
					createMultisigOperation(0, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', options.expectedMetadata)
				]);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal(['TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW']);
			};

			const addMultisigTests = (version, transactionMetadata) => {
				it(`can parse additions (${version})`, () => assertCanParse({
					version,
					transactionMetadata,
					modificationTypes: [1, 1, 1, 1],
					expectedMetadata: {
						addressAdditions: [
							'TBALNEMNEMKIMWLF65HTUWMQVX5G55EBBIWS4WQC',
							'TDONALICE7O3L63AS3KNDCPT7ZA7HMQTFZGYUCAH',
							'TALICE6KJ2SRSIJFVVFFH6ICUIYZ2ZZGNFUDJGRT',
							'TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV'
						],
						...transactionMetadata
					}
				}));

				it(`can parse deletions (${version})`, () => assertCanParse({
					version,
					transactionMetadata,
					modificationTypes: [2, 2, 2, 2],
					expectedMetadata: {
						addressDeletions: [
							'TBALNEMNEMKIMWLF65HTUWMQVX5G55EBBIWS4WQC',
							'TDONALICE7O3L63AS3KNDCPT7ZA7HMQTFZGYUCAH',
							'TALICE6KJ2SRSIJFVVFFH6ICUIYZ2ZZGNFUDJGRT',
							'TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV'
						],
						...transactionMetadata
					}
				}));

				it(`can parse mixed additions and deletions (${version})`, () => assertCanParse({
					version,
					transactionMetadata,
					modificationTypes: [1, 2, 1, 2],
					expectedMetadata: {
						addressAdditions: [
							'TDONALICE7O3L63AS3KNDCPT7ZA7HMQTFZGYUCAH',
							'TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV'
						],
						addressDeletions: [
							'TBALNEMNEMKIMWLF65HTUWMQVX5G55EBBIWS4WQC',
							'TALICE6KJ2SRSIJFVVFFH6ICUIYZ2ZZGNFUDJGRT'
						],
						...transactionMetadata
					}
				}));
			};

			addMultisigTests('v1', {});
			addMultisigTests('v2', { minApprovalDelta: 3 });
		});

		// endregion

		// region supply change

		describe('supply change', () => {
			const assertCanParse = async (action, expectedAmount) => {
				// Arrange:
				const textEncoder = new TextEncoder();
				const facade = new NemFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type: 'mosaic_supply_change_transaction_v1',
					signerPublicKey: '9822CF9571A5551EC19720B87A567A20797B75EC4B6711387643FC352FEF704E',
					mosaicId: { namespaceId: { name: textEncoder.encode('foo') }, name: textEncoder.encode('bar') },
					action,
					delta: 24680
				});

				const parser = createDefaultParser(facade.network);

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, transaction);

				// Assert:
				expect(operations).to.deep.equal([
					createTransferOperation(0, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', expectedAmount, 'foo.bar', 3)
				]);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal(['TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW']);
			};

			it('can parse increase', () => assertCanParse(models.MosaicSupplyChangeAction.INCREASE, '24680'));

			it('can parse decrease', () => assertCanParse(models.MosaicSupplyChangeAction.DECREASE, '-24680'));
		});

		// endregion

		// region namespace

		describe('namespace', () => {
			it('can parse', async () => {
				// Arrange:
				const facade = new NemFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type: 'namespace_registration_transaction_v1',
					signerPublicKey: '9822CF9571A5551EC19720B87A567A20797B75EC4B6711387643FC352FEF704E',
					rentalFeeSink: 'TAMESPACEWH4MKFMBCVFERDPOOP4FK7MTDJEYP35',
					rentalFee: 50000,
					name: 'roger'
				});

				const parser = createDefaultParser(facade.network);

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, transaction);

				// Assert:
				expect(operations).to.deep.equal([
					createTransferOperation(0, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '-50000', 'currency.fee', 2),
					createTransferOperation(1, 'TAMESPACEWH4MKFMBCVFERDPOOP4FK7MTDJEYP35', '50000', 'currency.fee', 2)
				]);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal(['TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW']);
			});
		});

		// endregion

		// region mosaic

		describe('mosaic', () => {
			const assertParse = async options => {
				// Arrange:
				const textEncoder = new TextEncoder();
				const facade = new NemFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type: 'mosaic_definition_transaction_v1',
					signerPublicKey: '9822CF9571A5551EC19720B87A567A20797B75EC4B6711387643FC352FEF704E',
					rentalFeeSink: 'TBMOSAICOD4F54EE5CDMR23CCBGOAM2XSJBR5OLC',
					rentalFee: 50000,

					mosaicDefinition: {
						ownerPublicKey: '9822CF9571A5551EC19720B87A567A20797B75EC4B6711387643FC352FEF704E',
						id: { namespaceId: { name: textEncoder.encode('foo') }, name: textEncoder.encode('bar') },
						properties: options.properties
					}
				});

				const parser = createDefaultParser(facade.network);

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, transaction);

				// Assert:
				expect(operations).to.deep.equal([
					createTransferOperation(0, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '-50000', 'currency.fee', 2),
					createTransferOperation(1, 'TBMOSAICOD4F54EE5CDMR23CCBGOAM2XSJBR5OLC', '50000', 'currency.fee', 2),
					...options.additionalOperations
				]);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal(['TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW']);
			};

			it('can parse without initial supply', () => assertParse({
				properties: [],
				additionalOperations: []
			}));

			it('can parse with initial supply and default divisibility', async () => {
				const textEncoder = new TextEncoder();
				await assertParse({
					properties: [
						{ property: { name: textEncoder.encode('initialSupply'), value: textEncoder.encode('123000') } },
						{ property: { name: textEncoder.encode('supplyMutable'), value: textEncoder.encode('false') } }
					],
					additionalOperations: [
						createTransferOperation(2, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '123000', 'foo.bar', 0)
					]
				});
			});

			it('can parse with initial supply and custom divisibility', async () => {
				const textEncoder = new TextEncoder();
				await assertParse({
					properties: [
						{ property: { name: textEncoder.encode('divisibility'), value: textEncoder.encode('4') } },
						{ property: { name: textEncoder.encode('initialSupply'), value: textEncoder.encode('123000') } },
						{ property: { name: textEncoder.encode('supplyMutable'), value: textEncoder.encode('false') } }
					],
					additionalOperations: [
						createTransferOperation(2, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '123000', 'foo.bar', 4)
					]
				});
			});
		});

		// endregion

		// region other

		describe('other', () => {
			const assertCanParse = async (options, expectedOperations) => {
				// Arrange:
				const facade = new NemFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type: 'account_key_link_transaction_v1',
					signerPublicKey: '9822CF9571A5551EC19720B87A567A20797B75EC4B6711387643FC352FEF704E',
					linkAction: 'link',
					remotePublicKey: 'BE0B4CF546B7B4F4BBFCFF9F574FDA527C07A53D3FC76F8BB7DB746F8E8E0A9F',
					fee: 123456
				});

				const parser = createDefaultParser(facade.network, options);

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, transaction);

				// Assert:
				expect(operations).to.deep.equal(expectedOperations);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal(['TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW']);
			};

			it('can parse', () => assertCanParse({}, []));

			it('can parse with fee', () => assertCanParse({ includeFeeOperation: true }, [
				createTransferOperation(0, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '-123456', 'currency.fee', 2)
			]));

			it('can parse with fee and operation status', () => assertCanParse(
				{ includeFeeOperation: true, operationStatus: 'success' },
				[
					setOperationStatus(createTransferOperation(0, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '-123456', 'currency.fee', 2))
				]
			));
		});

		// endregion

		// region aggregate

		describe('aggregate', () => {
			const assertCanParse = async options => {
				// Arrange:
				const facade = new NemFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type: 'multisig_transaction_v1',
					signerPublicKey: 'E5F290755F021258ACE3CB29452BF38B322D76F62CAF6E9D2A89B48ABF7DD778',
					fee: 8000,

					innerTransaction: facade.transactionFactory.create({
						type: 'transfer_transaction_v1',
						signerPublicKey: '9822CF9571A5551EC19720B87A567A20797B75EC4B6711387643FC352FEF704E',
						recipientAddress: 'TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV',
						amount: 12345_000000,
						fee: 700
					}),

					cosignatures: options.cosignatures.map(cosignatureDescriptor => {
						const cosignature = new models.SizePrefixedCosignatureV1();
						cosignature.cosignature = facade.transactionFactory.create({
							type: 'cosignature_v1',
							...cosignatureDescriptor
						});
						return cosignature;
					})
				});

				const parser = createDefaultParser(facade.network, options.parserOptions);

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, transaction);

				// Assert:
				expect(operations).to.deep.equal([
					createTransferOperation(0, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '-12345000000', 'currency.fee', 2),
					createTransferOperation(1, 'TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV', '12345000000', 'currency.fee', 2),
					createCosignOperation(2, 'TBALNEMNEMKIMWLF65HTUWMQVX5G55EBBIWS4WQC'),
					...options.additionalOperations
				]);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal(options.expectedSignerAddresses);
			};

			it('can parse', () => assertCanParse({
				cosignatures: [],
				additionalOperations: [],
				expectedSignerAddresses: ['TBALNEMNEMKIMWLF65HTUWMQVX5G55EBBIWS4WQC']
			}));

			it('can parse with explicit cosigners', () => assertCanParse({
				cosignatures: [
					{ signerPublicKey: '5D2EC9959153F54E5225EBBC6A677AF37DCB8C3968558F366B2841F9DA9CA14F', fee: 60 },
					{ signerPublicKey: '45880194FAD01FCB55887B73EEFFDC263914ED5749BF2F3ACB928C843C57BD9A', fee: 5 }
				],
				additionalOperations: [
					createCosignOperation(3, 'TBGJAGUAQY47BULYL4GRYBJLOI6XKXPJUXU25JRJ'),
					createCosignOperation(4, 'TDONALICE7O3L63AS3KNDCPT7ZA7HMQTFZGYUCAH')
				],
				expectedSignerAddresses: [
					'TBALNEMNEMKIMWLF65HTUWMQVX5G55EBBIWS4WQC',
					'TBGJAGUAQY47BULYL4GRYBJLOI6XKXPJUXU25JRJ',
					'TDONALICE7O3L63AS3KNDCPT7ZA7HMQTFZGYUCAH'
				]
			}));

			it('can parse with explicit cosigners with fee', () => assertCanParse({
				parserOptions: { includeFeeOperation: true },
				cosignatures: [
					{ signerPublicKey: '5D2EC9959153F54E5225EBBC6A677AF37DCB8C3968558F366B2841F9DA9CA14F', fee: 60 },
					{ signerPublicKey: '45880194FAD01FCB55887B73EEFFDC263914ED5749BF2F3ACB928C843C57BD9A', fee: 5 }
				],
				additionalOperations: [
					createCosignOperation(3, 'TBGJAGUAQY47BULYL4GRYBJLOI6XKXPJUXU25JRJ'),
					createCosignOperation(4, 'TDONALICE7O3L63AS3KNDCPT7ZA7HMQTFZGYUCAH'),
					createTransferOperation(5, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '-8065', 'currency.fee', 2)
				],
				expectedSignerAddresses: [
					'TBALNEMNEMKIMWLF65HTUWMQVX5G55EBBIWS4WQC',
					'TBGJAGUAQY47BULYL4GRYBJLOI6XKXPJUXU25JRJ',
					'TDONALICE7O3L63AS3KNDCPT7ZA7HMQTFZGYUCAH'
				]
			}));
		});

		// endregion

		// region parseTransactionAsRosettaTransaction

		it('can parse as rosetta transaction', async () => {
			// Arrange:
			const facade = new NemFacade('testnet');
			const transaction = facade.transactionFactory.create({
				type: 'transfer_transaction_v1',
				signerPublicKey: '9822CF9571A5551EC19720B87A567A20797B75EC4B6711387643FC352FEF704E',
				recipientAddress: 'TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV',
				amount: 12345_000000
			});

			const parser = createDefaultParser(facade.network);

			// Act:
			const rosettaTransaction = await parser.parseTransactionAsRosettaTransaction(
				convertTransactionSdkJsonToRestJson(transaction.toJson()),
				{ hash: { data: '7b7a5e55e3f788c036b759b6ad46ff91a67dc956bb4b360587f366397f251c62' } }
			);

			// Assert:
			expect(rosettaTransaction).to.deep.equal(new Transaction(
				new TransactionIdentifier('7B7A5E55E3F788C036B759B6AD46FF91A67DC956BB4B360587F366397F251C62'),
				[
					createTransferOperation(0, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '-12345000000', 'currency.fee', 2),
					createTransferOperation(1, 'TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV', '12345000000', 'currency.fee', 2)
				]
			));
		});

		// endregion
	});

	// region convertTransactionSdkJsonToRestJson

	describe('convertTransactionSdkJsonToRestJson', () => {
		it('can fixup mosaics array', () => {
			// Arrange:
			const textEncoder = new TextEncoder();
			const hexEncodeUtfString = name => utils.uint8ToHex(textEncoder.encode(name));

			// Act:
			const restJson = convertTransactionSdkJsonToRestJson({
				mosaics: [
					{
						mosaic: {
							mosaicId: { namespaceId: { name: hexEncodeUtfString('baz') }, name: hexEncodeUtfString('baz') },
							amount: 11111_000000
						}
					},
					{
						mosaic: {
							mosaicId: { namespaceId: { name: hexEncodeUtfString('nem') }, name: hexEncodeUtfString('xem') },
							amount: 12345_000000
						}
					},
					{
						mosaic: {
							mosaicId: { namespaceId: { name: hexEncodeUtfString('foo') }, name: hexEncodeUtfString('bar') },
							amount: 224433_000000
						}
					}
				]
			});

			// Assert:
			expect(restJson).to.deep.equal({
				mosaics: [
					{
						quantity: 11111_000000,
						mosaicId: { namespaceId: 'baz', name: 'baz' }
					},
					{
						quantity: 12345_000000,
						mosaicId: { namespaceId: 'nem', name: 'xem' }
					},
					{
						quantity: 224433_000000,
						mosaicId: { namespaceId: 'foo', name: 'bar' }
					}
				]
			});
		});

		it('can fixup mosaicId object', () => {
			// Arrange:
			const textEncoder = new TextEncoder();
			const hexEncodeUtfString = name => utils.uint8ToHex(textEncoder.encode(name));

			// Act:
			const restJson = convertTransactionSdkJsonToRestJson({
				mosaicId: { namespaceId: { name: hexEncodeUtfString('foo') }, name: hexEncodeUtfString('bar') }
			});

			// Assert:
			expect(restJson).to.deep.equal({
				mosaicId: { namespaceId: 'foo', name: 'bar' }
			});
		});

		it('can fixup modifications array', () => {
			// Act:
			const restJson = convertTransactionSdkJsonToRestJson({
				modifications: [
					{
						modification: {
							modificationType: 1,
							cosignatoryPublicKey: 'CB3F9F63C3F22DDA6D9BCD367476927426192A0497BC8AE38455D19ECB10DCC9'
						}
					},
					{
						modification: {
							modificationType: 2,
							cosignatoryPublicKey: '727643FB1D18214334C11280DF986A0AFEE128FC1F8BE7F9118E89C417E07771'
						}
					},
					{
						modification: {
							modificationType: 1,
							cosignatoryPublicKey: '45880194FAD01FCB55887B73EEFFDC263914ED5749BF2F3ACB928C843C57BD9A'
						}
					}
				]
			});

			// Assert:
			expect(restJson).to.deep.equal({
				modifications: [
					{
						modificationType: 1,
						cosignatoryPublicKey: 'CB3F9F63C3F22DDA6D9BCD367476927426192A0497BC8AE38455D19ECB10DCC9'
					},
					{
						modificationType: 2,
						cosignatoryPublicKey: '727643FB1D18214334C11280DF986A0AFEE128FC1F8BE7F9118E89C417E07771'
					},
					{
						modificationType: 1,
						cosignatoryPublicKey: '45880194FAD01FCB55887B73EEFFDC263914ED5749BF2F3ACB928C843C57BD9A'
					}
				]
			});
		});

		it('can fixup signerPublicKey property', () => {
			// Act:
			const restJson = convertTransactionSdkJsonToRestJson({
				signerPublicKey: 'CB3F9F63C3F22DDA6D9BCD367476927426192A0497BC8AE38455D19ECB10DCC9'
			});

			// Assert:
			expect(restJson).to.deep.equal({
				signer: 'CB3F9F63C3F22DDA6D9BCD367476927426192A0497BC8AE38455D19ECB10DCC9'
			});
		});

		it('can fixup innerTransaction object', () => {
			// Act:
			const restJson = convertTransactionSdkJsonToRestJson({
				innerTransaction: {
					type: 'transfer_transaction_v1',
					signerPublicKey: '9822CF9571A5551EC19720B87A567A20797B75EC4B6711387643FC352FEF704E'
				}
			});

			// Assert:
			expect(restJson).to.deep.equal({
				otherTrans: {
					type: 'transfer_transaction_v1',
					signer: '9822CF9571A5551EC19720B87A567A20797B75EC4B6711387643FC352FEF704E'
				}
			});
		});

		it('can fixup cosignatures array', () => {
			// Act:
			const restJson = convertTransactionSdkJsonToRestJson({
				cosignatures: [
					{ cosignature: { signerPublicKey: '727643FB1D18214334C11280DF986A0AFEE128FC1F8BE7F9118E89C417E07771', fee: 60 } },
					{ cosignature: { signerPublicKey: '45880194FAD01FCB55887B73EEFFDC263914ED5749BF2F3ACB928C843C57BD9A', fee: 5 } }
				]
			});

			// Assert:
			expect(restJson).to.deep.equal({
				signatures: [
					{ signer: '727643FB1D18214334C11280DF986A0AFEE128FC1F8BE7F9118E89C417E07771', fee: 60 },
					{ signer: '45880194FAD01FCB55887B73EEFFDC263914ED5749BF2F3ACB928C843C57BD9A', fee: 5 }
				]
			});
		});

		it('can fixup recipientAddress property', () => {
			// Act:
			const restJson = convertTransactionSdkJsonToRestJson({
				recipientAddress: '54414C494333334C514D50433344483733543559353253535645324C524853475242474F344B4956'
			});

			// Assert:
			expect(restJson).to.deep.equal({
				recipient: 'TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV'
			});
		});

		it('can fixup minApprovalDelta property', () => {
			// Act:
			const restJson = convertTransactionSdkJsonToRestJson({
				minApprovalDelta: 2
			});

			// Assert:
			expect(restJson).to.deep.equal({
				minCosignatories: { relativeChange: 2 }
			});
		});

		it('can fixup mosaicDefinition object and fee sink properties', () => {
			// Arrange:
			const textEncoder = new TextEncoder();
			const hexEncodeUtfString = name => utils.uint8ToHex(textEncoder.encode(name));

			// Act:
			const restJson = convertTransactionSdkJsonToRestJson({
				rentalFeeSink: 'TBMOSAICOD4F54EE5CDMR23CCBGOAM2XSJBR5OLC',
				rentalFee: 50000,

				mosaicDefinition: {
					ownerPublicKey: '9822CF9571A5551EC19720B87A567A20797B75EC4B6711387643FC352FEF704E',
					id: { namespaceId: { name: hexEncodeUtfString('foo') }, name: hexEncodeUtfString('bar') },
					properties: [
						{ property: { name: hexEncodeUtfString('initialSupply'), value: hexEncodeUtfString('123000') } },
						{ property: { name: hexEncodeUtfString('supplyMutable'), value: hexEncodeUtfString('false') } }
					]
				}
			});

			// Assert:
			expect(restJson).to.deep.equal({
				creationFeeSink: 'TBMOSAICOD4F54EE5CDMR23CCBGOAM2XSJBR5OLC',
				creationFee: 50000,

				mosaicDefinition: {
					ownerPublicKey: '9822CF9571A5551EC19720B87A567A20797B75EC4B6711387643FC352FEF704E',
					id: { namespaceId: 'foo', name: 'bar' },
					properties: [
						{ name: 'initialSupply', value: '123000' },
						{ name: 'supplyMutable', value: 'false' }
					]
				}
			});
		});

		it('does not fixup fee sink properties when mosaicDefinition is not present', () => {
			// Act:
			const restJson = convertTransactionSdkJsonToRestJson({
				rentalFeeSink: 'TBMOSAICOD4F54EE5CDMR23CCBGOAM2XSJBR5OLC',
				rentalFee: 50000
			});

			// Assert:
			expect(restJson).to.deep.equal({
				rentalFeeSink: 'TBMOSAICOD4F54EE5CDMR23CCBGOAM2XSJBR5OLC',
				rentalFee: 50000
			});
		});
	});

	// endregion
});

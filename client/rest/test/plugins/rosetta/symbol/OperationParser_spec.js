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

import PayloadResultVerifier from './utils/PayloadResultVerifier.js';
import Currency from '../../../../src/plugins/rosetta/openApi/model/Currency.js';
import Transaction from '../../../../src/plugins/rosetta/openApi/model/Transaction.js';
import TransactionIdentifier from '../../../../src/plugins/rosetta/openApi/model/TransactionIdentifier.js';
import { OperationParser, convertTransactionSdkJsonToRestJson } from '../../../../src/plugins/rosetta/symbol/OperationParser.js';
import { RosettaOperationFactory } from '../utils/rosettaTestUtils.js';
import { expect } from 'chai';
import { utils } from 'symbol-sdk';
import {
	Address, SymbolFacade, generateMosaicAliasId, models
} from 'symbol-sdk/symbol';

describe('Symbol OperationParser', () => {
	// region test accounts

	// address => public key mapping (sorted by public key)
	// TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ => 3119DA3BFF57385BB6F051B8A454F219CE519D28E50D5653F5F457486E9E8623
	// TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI => 527068DA90B142D98D27FF9BA2103A54230E3C8FAC8529E804123D986CACDCC9
	// TCJEJJBKDI62U4ZMO4VI7YAUVJE4STVCOBDSHXQ => 8A18D72A3D90547C9A2503C8513BC2A54D52B96119F4A5DF6E65CDA813EE5D9F
	// TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ => 93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690D7
	// TBOLYS45WU57DODH44O4ZUFA2ZJN34WT2ZPIDXY => B6C7F7C9BC0CD2845BE60679C17CA342F8E21AEE8A0881DA639E082DE1C0EDBF
	// TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI => ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6

	// endregion

	// region utils

	const {
		createCosignOperation, createMultisigOperation, createTransferOperation, setOperationStatus
	} = RosettaOperationFactory;

	const encodeParitalDecodedAddress = address => new Address(utils.hexToUint8(address.padEnd(48, '0'))).toString();

	const lookupCurrencySync = (mosaicId, transactionLocation) => {
		if ('currencyMosaicId' === mosaicId)
			return new Currency('currency.fee', 2);

		if (mosaicId === generateMosaicAliasId('foo.bar'))
			return new Currency('foo.bar', 3);

		if (mosaicId === generateMosaicAliasId('baz.bar'))
			return new Currency('baz.bar', 1);

		if (mosaicId === generateMosaicAliasId('check.location')) {
			const currencyName = undefined === transactionLocation
				? 'undefined'
				: `${transactionLocation.height}.${transactionLocation.primaryId}.${transactionLocation.secondaryId}`;
			return new Currency(currencyName, 4);
		}

		return new Currency('symbol.xym', 6);
	};

	const resolveAddressDefault = (address, transactionLocation) => {
		const encodedTransactionLocation = transactionLocation
			? [transactionLocation.height, transactionLocation.primaryId, transactionLocation.secondaryId]
				.map(value => value.toString(16).padStart(4, '0').toUpperCase())
				.join('')
			: '';
		const resolutionMap = {
			'999234567890ABCDEA000000000000000000000000000000': '98D1ACBC1FA7F463D3591BF82E27D3EE18592EC4B71D9829',
			'999234567890ABCDEF000000000000000000000000000000': '985F73D67009A33962EFE43BD21288AFF7428B43FE942B5E',
			'999234567890ABFFFF000000000000000000000000000000': `98${encodedTransactionLocation}`.padEnd(48, '0')
		};

		return Promise.resolve(resolutionMap[address] || address);
	};

	const createDefaultParser = (network, additionalOptions = {}) => new OperationParser(network, {
		lookupCurrency: (...args) => Promise.resolve(lookupCurrencySync(...args)),
		resolveAddress: resolveAddressDefault,
		...additionalOptions
	});

	const parseTransaction = (parser, transaction, metadata = undefined) =>
		parser.parseTransaction(convertTransactionSdkJsonToRestJson(transaction.toJson()), metadata);

	// endregion

	// region createFromServices

	describe('createFromServices', () => {
		it('can create parser', () => {
			// Act:
			const parser = OperationParser.createFromServices({
				config: {
					network: { name: 'testnet' }
				},
				proxy: {}
			});

			// Assert:
			expect(parser).to.not.equal(undefined);
		});
	});

	// endregion

	describe('transaction', () => {
		// region transfer

		describe('transfer', () => {
			it('can parse with single mosaic', async () => {
				// Arrange:
				const facade = new SymbolFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type: 'transfer_transaction_v1',
					signerPublicKey: '527068DA90B142D98D27FF9BA2103A54230E3C8FAC8529E804123D986CACDCC9',
					recipientAddress: 'TGJDIVTYSCV433YAAAAAAAAAAAAAAAAAAAAAAAA',
					mosaics: [
						{ mosaicId: generateMosaicAliasId('symbol.xym'), amount: 12345_000000n }
					]
				});

				const parser = createDefaultParser(facade.network);

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, transaction);

				// Assert:
				expect(operations).to.deep.equal([
					createTransferOperation(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '-12345000000', 'symbol.xym', 6),
					createTransferOperation(1, 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ', '12345000000', 'symbol.xym', 6)
				]);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal(['TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI']);
			});

			const assertCanParseMultipleMosaic = async options => {
				// Arrange:
				const facade = new SymbolFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type: 'transfer_transaction_v1',
					signerPublicKey: '527068DA90B142D98D27FF9BA2103A54230E3C8FAC8529E804123D986CACDCC9',
					recipientAddress: options.unresolvedRecipientAddress,
					mosaics: [
						{ mosaicId: generateMosaicAliasId('baz.bar'), amount: 22222 },
						{ mosaicId: generateMosaicAliasId(options.unresolvedMosaicName), amount: 12345n },
						{ mosaicId: generateMosaicAliasId('foo.bar'), amount: 1000 }
					]
				});

				const parser = createDefaultParser(facade.network);

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, transaction, options.metadata);

				// Assert:
				expect(operations).to.deep.equal([
					createTransferOperation(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '-22222', 'baz.bar', 1),
					createTransferOperation(1, options.resolvedRecipientAddress, '22222', 'baz.bar', 1),
					createTransferOperation(2, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '-12345', ...options.resolvedMosaicProperties),
					createTransferOperation(3, options.resolvedRecipientAddress, '12345', ...options.resolvedMosaicProperties),
					createTransferOperation(4, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '-1000', 'foo.bar', 3),
					createTransferOperation(5, options.resolvedRecipientAddress, '1000', 'foo.bar', 3)
				]);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal(['TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI']);
			};

			it('can parse with multiple mosaic', () => assertCanParseMultipleMosaic({
				metadata: undefined,
				unresolvedRecipientAddress: 'TGJDIVTYSCV433YAAAAAAAAAAAAAAAAAAAAAAAA',
				resolvedRecipientAddress: 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ',
				unresolvedMosaicName: 'symbol.xym',
				resolvedMosaicProperties: ['symbol.xym', 6]
			}));

			it('is transaction location aware', () => assertCanParseMultipleMosaic({
				metadata: { height: '1234', index: 11 },
				unresolvedRecipientAddress: 'TGJDIVTYSCV777YAAAAAAAAAAAAAAAAAAAAAAAA',
				resolvedRecipientAddress: encodeParitalDecodedAddress('9804D2000C0000'),
				unresolvedMosaicName: 'check.location',
				resolvedMosaicProperties: ['1234.12.0', 4]
			}));

			it('filters out zero transfers', async () => {
				// Arrange:
				const facade = new SymbolFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type: 'transfer_transaction_v1',
					signerPublicKey: '527068DA90B142D98D27FF9BA2103A54230E3C8FAC8529E804123D986CACDCC9',
					recipientAddress: 'TGJDIVTYSCV433YAAAAAAAAAAAAAAAAAAAAAAAA',
					mosaics: [
						{ mosaicId: generateMosaicAliasId('baz.bar'), amount: 22222 },
						{ mosaicId: generateMosaicAliasId('symbol.xym'), amount: 0 },
						{ mosaicId: generateMosaicAliasId('foo.bar'), amount: 1000 }
					]
				});

				const parser = createDefaultParser(facade.network);

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, transaction);

				// Assert:
				expect(operations).to.deep.equal([
					createTransferOperation(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '-22222', 'baz.bar', 1),
					createTransferOperation(1, 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ', '22222', 'baz.bar', 1),
					createTransferOperation(2, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '-1000', 'foo.bar', 3),
					createTransferOperation(3, 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ', '1000', 'foo.bar', 3)
				]);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal(['TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI']);
			});
		});

		// endregion

		// region multisig

		describe('multisig', () => {
			const assertCanParse = async options => {
				// Arrange:
				const transactionMetadata = {
					minApprovalDelta: 1,
					minRemovalDelta: 2,
					[options.propertyName]: [
						'TCIO5J4WTXCVC76XZPBWHHNAD2NU52U2MOOVN4Q',
						options.unresolvedAddress,
						'TBO3SFA2XM3HY5QPBT7C6CCY4K6VXTHIQGPG6OQ'
					]
				};

				const facade = new SymbolFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type: 'multisig_account_modification_transaction_v1',
					signerPublicKey: '527068DA90B142D98D27FF9BA2103A54230E3C8FAC8529E804123D986CACDCC9',
					...transactionMetadata
				});

				const parser = createDefaultParser(facade.network);

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, transaction, options.metadata);

				// Assert:
				expect(operations).to.deep.equal([
					createMultisigOperation(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', {
						...transactionMetadata,
						[options.propertyName]: [ // resolved addresses
							'TCIO5J4WTXCVC76XZPBWHHNAD2NU52U2MOOVN4Q',
							options.resolvedAddress,
							'TBO3SFA2XM3HY5QPBT7C6CCY4K6VXTHIQGPG6OQ'
						]
					})
				]);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal(['TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI']);
			};

			const assertCanParseNoLocation = propertyName => assertCanParse({
				propertyName,
				metadata: undefined,
				unresolvedAddress: 'TGJDIVTYSCV433YAAAAAAAAAAAAAAAAAAAAAAAA',
				resolvedAddress: 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ'
			});

			it('can parse additions', () => assertCanParseNoLocation('addressAdditions'));

			it('can parse deletions', () => assertCanParseNoLocation('addressDeletions'));

			const assertCanParseLocationAware = propertyName => assertCanParse({
				propertyName,
				metadata: { height: '1234', index: 11 },
				unresolvedAddress: 'TGJDIVTYSCV777YAAAAAAAAAAAAAAAAAAAAAAAA',
				resolvedAddress: encodeParitalDecodedAddress('9804D2000C0000')
			});

			it('is transaction location aware (additions)', () => assertCanParseLocationAware('addressAdditions'));

			it('is transaction location aware (deletions)', () => assertCanParseLocationAware('addressDeletions'));
		});

		// endregion

		// region supply change

		describe('supply change', () => {
			const assertCanParse = async options => {
				// Arrange:
				const facade = new SymbolFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type: 'mosaic_supply_change_transaction_v1',
					signerPublicKey: '527068DA90B142D98D27FF9BA2103A54230E3C8FAC8529E804123D986CACDCC9',
					mosaicId: generateMosaicAliasId(options.unresolvedMosaicName),
					action: options.action,
					delta: 24680n
				});

				const parser = createDefaultParser(facade.network);

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, transaction, options.metadata);

				// Assert:
				expect(operations).to.deep.equal([
					createTransferOperation(
						0,
						'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI',
						options.expectedAmount,
						...options.resolvedMosaicProperties
					)
				]);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal(['TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI']);
			};

			const assertCanParseNoLocation = async (action, expectedAmount) => assertCanParse({
				action,
				expectedAmount,
				metadata: undefined,
				unresolvedMosaicName: 'foo.bar',
				resolvedMosaicProperties: ['foo.bar', 3]
			});

			it('can parse increase', () => assertCanParseNoLocation(models.MosaicSupplyChangeAction.INCREASE, '24680'));

			it('can parse decrease', () => assertCanParseNoLocation(models.MosaicSupplyChangeAction.DECREASE, '-24680'));

			it('is transaction location aware', () => assertCanParse({
				action: models.MosaicSupplyChangeAction.INCREASE,
				expectedAmount: '24680',
				metadata: { height: '1234', index: 11 },
				unresolvedMosaicName: 'check.location',
				resolvedMosaicProperties: ['1234.12.0', 4]
			}));
		});

		// endregion

		// region supply revocation

		describe('supply revocation', () => {
			const assertCanParse = async options => {
				// Arrange:
				const facade = new SymbolFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type: 'mosaic_supply_revocation_transaction_v1',
					signerPublicKey: '527068DA90B142D98D27FF9BA2103A54230E3C8FAC8529E804123D986CACDCC9',
					sourceAddress: options.unresolvedSourceAddress,
					mosaic: {
						mosaicId: generateMosaicAliasId(options.unresolvedMosaicName),
						amount: 24680n
					}
				});

				const parser = createDefaultParser(facade.network);

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, transaction, options.metadata);

				// Assert:
				expect(operations).to.deep.equal([
					createTransferOperation(0, options.resolvedSourceAddress, '-24680', ...options.resolvedMosaicProperties),
					createTransferOperation(1, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '24680', ...options.resolvedMosaicProperties)
				]);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal(['TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI']);
			};

			it('can parse', () => assertCanParse({
				metadata: undefined,
				unresolvedSourceAddress: 'TGJDIVTYSCV433YAAAAAAAAAAAAAAAAAAAAAAAA',
				resolvedSourceAddress: 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ',
				unresolvedMosaicName: 'foo.bar',
				resolvedMosaicProperties: ['foo.bar', 3]
			}));

			it('is transaction location aware', () => assertCanParse({
				metadata: { height: '1234', index: 11 },
				unresolvedSourceAddress: 'TGJDIVTYSCV777YAAAAAAAAAAAAAAAAAAAAAAAA',
				resolvedSourceAddress: encodeParitalDecodedAddress('9804D2000C0000'),
				unresolvedMosaicName: 'check.location',
				resolvedMosaicProperties: ['1234.12.0', 4]
			}));
		});

		// endregion

		// region other

		describe('other', () => {
			const assertCanParse = async (options, metadata, expectedOperations) => {
				// Arrange:
				const facade = new SymbolFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type: 'mosaic_definition_transaction_v1',
					signerPublicKey: '527068DA90B142D98D27FF9BA2103A54230E3C8FAC8529E804123D986CACDCC9',
					duration: 1n,
					nonce: 123,
					flags: 'transferable restrictable',
					divisibility: 2,
					fee: 123456
				});

				const parser = createDefaultParser(facade.network, options);

				// Act: add size to emulate REST JSON
				const { operations, signerAddresses } = await parser.parseTransaction(
					{ ...convertTransactionSdkJsonToRestJson(transaction.toJson()), size: 100 },
					metadata
				);

				// Assert:
				expect(operations).to.deep.equal(expectedOperations);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal(['TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI']);
			};

			it('can parse', () => assertCanParse({}, {}, []));

			it('can parse with max fee (unconfirmed)', () => assertCanParse({ includeFeeOperation: true }, {}, [
				createTransferOperation(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '-123456', 'currency.fee', 2)
			]));

			it('can parse with fee (confirmed)', () => assertCanParse({ includeFeeOperation: true }, { feeMultiplier: 200 }, [
				createTransferOperation(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '-20000', 'currency.fee', 2)
			]));

			it('can parse with fee (confirmed, zero)', () => assertCanParse({ includeFeeOperation: true }, { feeMultiplier: 0 }, [
			]));

			it('can parse with fee (confirmed) and operation status', () => assertCanParse(
				{ includeFeeOperation: true, operationStatus: 'success' },
				{ feeMultiplier: 200 },
				[
					setOperationStatus(createTransferOperation(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '-20000', 'currency.fee', 2))
				]
			));
		});

		// endregion

		// region aggregate

		describe('aggregate', () => {
			const createDefaultVerifier = cosignatureCount => {
				const verifier = new PayloadResultVerifier();
				verifier.addTransfer(
					'ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6',
					'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI',
					100n
				);
				verifier.addUnsupported('ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6');
				verifier.addMultisigModification('527068DA90B142D98D27FF9BA2103A54230E3C8FAC8529E804123D986CACDCC9', {
					minApprovalDelta: 1,
					minRemovalDelta: 2,
					addressAdditions: ['TCIO5J4WTXCVC76XZPBWHHNAD2NU52U2MOOVN4Q', 'TGJDIVTYSCV433YAAAAAAAAAAAAAAAAAAAAAAAA']
				});
				verifier.addTransfer(
					'93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690D7',
					'TGJDIVTYSCV432QAAAAAAAAAAAAAAAAAAAAAAAA',
					50n,
					'foo.bar'
				);
				verifier.addTransfer(
					'ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6',
					'TCJEJJBKDI62U4ZMO4VI7YAUVJE4STVCOBDSHXQ',
					33n
				);

				verifier.buildAggregate(
					'B6C7F7C9BC0CD2845BE60679C17CA342F8E21AEE8A0881DA639E082DE1C0EDBF',
					1001n + (60n * 60n * 1000n),
					cosignatureCount
				);

				return verifier;
			};

			const createDefaultVerifierOperations = () => ([
				createTransferOperation(0, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '-100', 'symbol.xym', 6),
				createTransferOperation(1, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '100', 'symbol.xym', 6),
				createMultisigOperation(2, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', {
					minApprovalDelta: 1,
					minRemovalDelta: 2,
					addressAdditions: ['TCIO5J4WTXCVC76XZPBWHHNAD2NU52U2MOOVN4Q', 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ']
				}),
				createTransferOperation(3, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', '-50', 'foo.bar', 3),
				createTransferOperation(4, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '50', 'foo.bar', 3),
				createTransferOperation(5, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '-33', 'symbol.xym', 6),
				createTransferOperation(6, 'TCJEJJBKDI62U4ZMO4VI7YAUVJE4STVCOBDSHXQ', '33', 'symbol.xym', 6)
			]);

			it('can parse multiple transactions', async () => {
				// Arrange:
				const verifier = createDefaultVerifier(1);

				const parser = createDefaultParser(verifier.facade.network);

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, verifier.aggregateTransaction);

				// Assert:
				expect(operations).to.deep.equal(createDefaultVerifierOperations());
				expect(signerAddresses.map(address => address.toString())).to.deep.equal([
					'TBOLYS45WU57DODH44O4ZUFA2ZJN34WT2ZPIDXY'
				]);
			});

			it('can parse multiple transactions with explicit cosigners', async () => {
				// Arrange:
				const verifier = createDefaultVerifier(2);

				verifier.addCosignature('3119DA3BFF57385BB6F051B8A454F219CE519D28E50D5653F5F457486E9E8623');
				verifier.addCosignature('93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690D7'); // redundant

				const parser = createDefaultParser(verifier.facade.network);

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, verifier.aggregateTransaction);

				// Assert:
				expect(operations).to.deep.equal([
					...createDefaultVerifierOperations(),
					createCosignOperation(7, 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ'),
					createCosignOperation(8, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ')
				]);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal([
					'TBOLYS45WU57DODH44O4ZUFA2ZJN34WT2ZPIDXY',
					'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ',
					'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ'
				]);
			});

			const assertCanParseWithFee = async (metadata, expectedFeeAmount) => {
				// Arrange:
				const verifier = createDefaultVerifier(2);

				verifier.addCosignature('3119DA3BFF57385BB6F051B8A454F219CE519D28E50D5653F5F457486E9E8623');
				verifier.addCosignature('93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690D7'); // redundant

				const parser = createDefaultParser(verifier.facade.network, {
					includeFeeOperation: true
				});

				// Act: add size to emulate REST JSON
				const { operations, signerAddresses } = await parser.parseTransaction({
					...convertTransactionSdkJsonToRestJson(verifier.aggregateTransaction.toJson()),
					size: 100
				}, metadata);

				// Assert:
				expect(operations).to.deep.equal([
					...createDefaultVerifierOperations(),
					createCosignOperation(7, 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ'),
					createCosignOperation(8, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ'),
					createTransferOperation(9, 'TBOLYS45WU57DODH44O4ZUFA2ZJN34WT2ZPIDXY', expectedFeeAmount, 'currency.fee', 2)
				]);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal([
					'TBOLYS45WU57DODH44O4ZUFA2ZJN34WT2ZPIDXY',
					'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ',
					'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ'
				]);
			};

			it('can parse multiple transactions with explicit cosigners including max fee (unconfirmed)', () =>
				assertCanParseWithFee({}, '-85680')); // (max) fee calculated by PayloadResultVerifier

			it('can parse multiple transactions with explicit cosigners including fee (confirmed)', () =>
				assertCanParseWithFee({ feeMultiplier: 200 }, '-20000'));
		});

		// endregion

		// region location

		describe('location', () => {
			const runTopLevelLocationTest = async (metadata, expectedResolutions) => {
				// Arrange:
				const facade = new SymbolFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type: 'transfer_transaction_v1',
					signerPublicKey: '527068DA90B142D98D27FF9BA2103A54230E3C8FAC8529E804123D986CACDCC9',
					recipientAddress: 'TGJDIVTYSCV777YAAAAAAAAAAAAAAAAAAAAAAAA',
					mosaics: [
						{ mosaicId: generateMosaicAliasId('check.location'), amount: 1000n }
					]
				});

				const parser = createDefaultParser(facade.network);

				// Act:
				const { operations } = await parseTransaction(parser, transaction, metadata);

				// Assert:
				expect(operations).to.deep.equal([
					createTransferOperation(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '-1000', expectedResolutions.mosaicName, 4),
					createTransferOperation(1, expectedResolutions.address, '1000', expectedResolutions.mosaicName, 4)
				]);
			};

			it('can pass down for top level transaction (valid)', () => runTopLevelLocationTest(
				{ height: '1234', index: 11 },
				{ mosaicName: '1234.12.0', address: encodeParitalDecodedAddress('9804D2000C0000') }
			));

			it('can pass down for top level transaction (height - undefined)', () => runTopLevelLocationTest(
				{ index: 11 },
				{ mosaicName: 'undefined', address: encodeParitalDecodedAddress('98000000000000') }
			));

			it('can pass down for top level transaction (height - zero)', () => runTopLevelLocationTest(
				{ height: '0', index: 11 },
				{ mosaicName: 'undefined', address: encodeParitalDecodedAddress('98000000000000') }
			));

			const runEmbeddedLocationTest = async (topLevelMetadata, subTransactionMetadatas, expectedResolutions) => {
				// Arrange:
				const verifier = new PayloadResultVerifier();
				verifier.addTransfer(
					'ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6',
					'TGJDIVTYSCV777YAAAAAAAAAAAAAAAAAAAAAAAA',
					100n,
					'check.location'
				);
				verifier.addTransfer(
					'93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690D7',
					'TGJDIVTYSCV777YAAAAAAAAAAAAAAAAAAAAAAAA',
					50n,
					'check.location'
				);
				verifier.addTransfer(
					'ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6',
					'TGJDIVTYSCV777YAAAAAAAAAAAAAAAAAAAAAAAA',
					33n,
					'check.location'
				);

				verifier.buildAggregate(
					'B6C7F7C9BC0CD2845BE60679C17CA342F8E21AEE8A0881DA639E082DE1C0EDBF',
					1001n + (60n * 60n * 1000n),
					1
				);

				const aggregateTransactionJson = convertTransactionSdkJsonToRestJson(verifier.aggregateTransaction.toJson());
				aggregateTransactionJson.transactions.forEach((transaction, i) => {
					transaction.meta = subTransactionMetadatas[i];
				});

				const parser = createDefaultParser(verifier.facade.network);

				// Act:
				const { operations } = await parser.parseTransaction(aggregateTransactionJson, topLevelMetadata);

				// Assert:
				expect(operations).to.deep.equal([
					createTransferOperation(0, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '-100', expectedResolutions[0].mosaicName, 4),
					createTransferOperation(1, expectedResolutions[0].address, '100', expectedResolutions[0].mosaicName, 4),
					createTransferOperation(2, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', '-50', expectedResolutions[1].mosaicName, 4),
					createTransferOperation(3, expectedResolutions[1].address, '50', expectedResolutions[1].mosaicName, 4),
					createTransferOperation(4, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '-33', expectedResolutions[2].mosaicName, 4),
					createTransferOperation(5, expectedResolutions[2].address, '33', expectedResolutions[2].mosaicName, 4)
				]);
			};

			it('can pass down for sub transaction (valid)', () => runEmbeddedLocationTest(
				{ height: '1234', index: 11 },
				[
					{ index: 4 },
					{ index: 7 },
					{ index: 3 }
				],
				[
					{ mosaicName: '1234.12.5', address: encodeParitalDecodedAddress('9804D2000C0005') },
					{ mosaicName: '1234.12.8', address: encodeParitalDecodedAddress('9804D2000C0008') },
					{ mosaicName: '1234.12.4', address: encodeParitalDecodedAddress('9804D2000C0004') }
				]
			));

			it('can pass down for sub transaction (undefined - top level)', () => runEmbeddedLocationTest(
				{ index: 11 },
				[
					{ index: 4 },
					{ index: 7 },
					{ index: 3 }
				],
				[
					{ mosaicName: 'undefined', address: encodeParitalDecodedAddress('98000000000000') },
					{ mosaicName: 'undefined', address: encodeParitalDecodedAddress('98000000000000') },
					{ mosaicName: 'undefined', address: encodeParitalDecodedAddress('98000000000000') }
				]
			));

			it('can pass down for sub transaction (undefined - sub level)', () => runEmbeddedLocationTest(
				{ height: '1234', index: 11 },
				[
					undefined,
					{ index: 7 },
					{}
				],
				[
					{ mosaicName: 'undefined', address: encodeParitalDecodedAddress('98000000000000') },
					{ mosaicName: '1234.12.8', address: encodeParitalDecodedAddress('9804D2000C0008') },
					{ mosaicName: 'undefined', address: encodeParitalDecodedAddress('98000000000000') }
				]
			));
		});

		// endregion

		// region parseTransactionAsRosettaTransaction

		it('can parse as rosetta transaction', async () => {
			// Arrange:
			const facade = new SymbolFacade('testnet');
			const transaction = facade.transactionFactory.create({
				type: 'transfer_transaction_v1',
				signerPublicKey: '527068DA90B142D98D27FF9BA2103A54230E3C8FAC8529E804123D986CACDCC9',
				recipientAddress: 'TGJDIVTYSCV433YAAAAAAAAAAAAAAAAAAAAAAAA',
				mosaics: [
					{ mosaicId: generateMosaicAliasId('symbol.xym'), amount: 12345_000000n }
				]
			});

			const parser = createDefaultParser(facade.network);

			// Act:
			const rosettaTransaction = await parser.parseTransactionAsRosettaTransaction(
				convertTransactionSdkJsonToRestJson(transaction.toJson()),
				{ hash: '7B7A5E55E3F788C036B759B6AD46FF91A67DC956BB4B360587F366397F251C62' }
			);

			// Assert:
			expect(rosettaTransaction).to.deep.equal(new Transaction(
				new TransactionIdentifier('7B7A5E55E3F788C036B759B6AD46FF91A67DC956BB4B360587F366397F251C62'),
				[
					createTransferOperation(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '-12345000000', 'symbol.xym', 6),
					createTransferOperation(1, 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ', '12345000000', 'symbol.xym', 6)
				]
			));
		});

		// endregion
	});

	// region receipt

	describe('receipt', () => {
		const runReceiptTest = async (receiptJson, expectedOperations) => {
			// Arrange:
			const facade = new SymbolFacade('testnet');

			const parser = createDefaultParser(facade.network);

			// Act:
			const { operations } = await parser.parseReceipt(convertTransactionSdkJsonToRestJson(receiptJson));

			// Assert:
			expect(operations).to.deep.equal(expectedOperations);
		};

		describe('debit', () => {
			const createReceiptJson = amount => ({
				type: 12626,
				targetAddress: '982390440A195B82D4A06810038B1400CE7CDA7AB3F48F99',
				mosaicId: generateMosaicAliasId('foo.bar'),
				amount
			});

			it('can parse', () => runReceiptTest(createReceiptJson('4741734'), [
				createTransferOperation(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '-4741734', 'foo.bar', 3)
			]));

			it('filters out zero amount', () => runReceiptTest(createReceiptJson('0'), []));
		});

		describe('credit', () => {
			const createReceiptJson = amount => ({
				type: 8515,
				targetAddress: '982390440A195B82D4A06810038B1400CE7CDA7AB3F48F99',
				mosaicId: generateMosaicAliasId('foo.bar'),
				amount
			});

			it('can parse', () => runReceiptTest(createReceiptJson('4741734'), [
				createTransferOperation(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '4741734', 'foo.bar', 3)
			]));

			it('filters out zero amount', () => runReceiptTest(createReceiptJson('0'), []));
		});

		describe('transfer', () => {
			const createReceiptJson = amount => ({
				type: 4942,
				senderAddress: '985F73D67009A33962EFE43BD21288AFF7428B43FE942B5E',
				recipientAddress: '982390440A195B82D4A06810038B1400CE7CDA7AB3F48F99',
				mosaicId: generateMosaicAliasId('foo.bar'),
				amount
			});

			it('can parse', () => runReceiptTest(createReceiptJson('4741734'), [
				createTransferOperation(0, 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ', '-4741734', 'foo.bar', 3),
				createTransferOperation(1, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '4741734', 'foo.bar', 3)
			]));

			it('filters out zero amount', () => runReceiptTest(createReceiptJson('0'), []));
		});

		describe('other', () => {
			const createReceiptJson = amount => ({
				type: 1111,
				targetAddress: '982390440A195B82D4A06810038B1400CE7CDA7AB3F48F99',
				mosaicId: generateMosaicAliasId('foo.bar'),
				amount
			});

			it('can parse', () => runReceiptTest(createReceiptJson('4741734'), []));

			it('can parse without amount', () => runReceiptTest(createReceiptJson(undefined), []));
		});
	});

	// endregion

	// region convertTransactionSdkJsonToRestJson

	describe('convertTransactionSdkJsonToRestJson', () => {
		it('can fixup mosaics array', () => {
			// Act:
			const restJson = convertTransactionSdkJsonToRestJson({
				mosaics: [
					{ mosaicId: generateMosaicAliasId('baz.bar').toString(), amount: '22222' },
					{ mosaicId: generateMosaicAliasId('symbol.xym').toString(), amount: '12345000000' },
					{ mosaicId: generateMosaicAliasId('foo.bar').toString(), amount: '1000' }
				]
			});

			// Assert:
			expect(restJson).to.deep.equal({
				mosaics: [
					{ id: 'AC5B81883DD40E08', amount: '22222' },
					{ id: 'E74B99BA41F4AFEE', amount: '12345000000' },
					{ id: 'EC673E105521B12F', amount: '1000' }
				]
			});
		});

		it('can fixup mosaic object', () => {
			// Act:
			const restJson = convertTransactionSdkJsonToRestJson({
				mosaic: { mosaicId: generateMosaicAliasId('baz.bar').toString(), amount: '22222' }
			});

			// Assert:
			expect(restJson).to.deep.equal({
				mosaicId: 'AC5B81883DD40E08',
				amount: '22222'
			});
		});

		it('can fixup mosaicId property', () => {
			// Act:
			const restJson = convertTransactionSdkJsonToRestJson({
				mosaicId: generateMosaicAliasId('baz.bar').toString(),
				amount: '22222'
			});

			// Assert:
			expect(restJson).to.deep.equal({
				mosaicId: 'AC5B81883DD40E08',
				amount: '22222'
			});
		});

		it('can fixup transactions array', () => {
			// Act:
			const restJson = convertTransactionSdkJsonToRestJson({
				transactions: [
					{
						mosaics: [
							{ mosaicId: generateMosaicAliasId('baz.bar').toString(), amount: '22222' },
							{ mosaicId: generateMosaicAliasId('symbol.xym').toString(), amount: '12345000000' },
							{ mosaicId: generateMosaicAliasId('foo.bar').toString(), amount: '1000' }
						]
					},
					{
						mosaicId: generateMosaicAliasId('baz.bar').toString(),
						amount: '33333'
					}
				]
			});

			// Assert:
			expect(restJson).to.deep.equal({
				transactions: [
					{
						transaction: {
							mosaics: [
								{ id: 'AC5B81883DD40E08', amount: '22222' },
								{ id: 'E74B99BA41F4AFEE', amount: '12345000000' },
								{ id: 'EC673E105521B12F', amount: '1000' }
							]
						}
					},
					{
						transaction: {
							mosaicId: 'AC5B81883DD40E08',
							amount: '33333'
						}
					}
				]
			});
		});

		it('can fixup fee property', () => {
			// Act:
			const restJson = convertTransactionSdkJsonToRestJson({
				fee: '12345'
			});

			// Assert:
			expect(restJson).to.deep.equal({
				maxFee: '12345'
			});
		});
	});

	// endregion
});

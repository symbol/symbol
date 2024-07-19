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
import { OperationParser, convertTransactionSdkJsonToRestJson } from '../../../src/plugins/rosetta/OperationParser.js';
import AccountIdentifier from '../../../src/plugins/rosetta/openApi/model/AccountIdentifier.js';
import Amount from '../../../src/plugins/rosetta/openApi/model/Amount.js';
import Currency from '../../../src/plugins/rosetta/openApi/model/Currency.js';
import Operation from '../../../src/plugins/rosetta/openApi/model/Operation.js';
import OperationIdentifier from '../../../src/plugins/rosetta/openApi/model/OperationIdentifier.js';
import Transaction from '../../../src/plugins/rosetta/openApi/model/Transaction.js';
import TransactionIdentifier from '../../../src/plugins/rosetta/openApi/model/TransactionIdentifier.js';
import { expect } from 'chai';
import { SymbolFacade, generateMosaicAliasId, models } from 'symbol-sdk/symbol';

describe('OperationParser', () => {
	// region utils

	const createTransferOperation = (index, address, amount, currencyName, currencyDecimals) => {
		const operation = new Operation(new OperationIdentifier(index), 'transfer');
		operation.account = new AccountIdentifier(address);
		operation.amount = new Amount(amount, new Currency(currencyName, currencyDecimals));
		operation.status = 'success';
		return operation;
	};

	const createMultisigOperation = (index, address, metadata) => {
		const operation = new Operation(new OperationIdentifier(index), 'multisig');
		operation.account = new AccountIdentifier(address);
		operation.metadata = {
			addressAdditions: [],
			addressDeletions: [],
			...metadata
		};
		operation.status = 'success';
		return operation;
	};

	const createCosignOperation = (index, address) => {
		const operation = new Operation(new OperationIdentifier(index), 'cosign');
		operation.account = new AccountIdentifier(address);
		operation.status = 'success';
		return operation;
	};

	const lookupCurrencyDefault = (mosaicId, transactionLocation) => {
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

	const parseTransaction = (parser, transaction, metadata = undefined) =>
		parser.parseTransaction(convertTransactionSdkJsonToRestJson(transaction.toJson()), metadata);

	// endregion

	describe('transaction', () => {
		// region transfer

		describe('transfer', () => {
			it('can parse with single mosaic', async () => {
				// Arrange:
				const facade = new SymbolFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type: 'transfer_transaction_v1',
					recipientAddress: 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ',
					signerPublicKey: '527068DA90B142D98D27FF9BA2103A54230E3C8FAC8529E804123D986CACDCC9',
					mosaics: [
						{ mosaicId: generateMosaicAliasId('symbol.xym'), amount: 12345_000000n }
					]
				});

				const parser = new OperationParser(facade.network, { lookupCurrency: lookupCurrencyDefault });

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, transaction);

				// Assert:
				expect(operations).to.deep.equal([
					createTransferOperation(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '-12345000000', 'symbol.xym', 6),
					createTransferOperation(1, 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ', '12345000000', 'symbol.xym', 6)
				]);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal(['TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI']);
			});

			it('can parse with multiple mosaic', async () => {
				// Arrange:
				const facade = new SymbolFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type: 'transfer_transaction_v1',
					recipientAddress: 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ',
					signerPublicKey: '527068DA90B142D98D27FF9BA2103A54230E3C8FAC8529E804123D986CACDCC9',
					mosaics: [
						{ mosaicId: generateMosaicAliasId('baz.bar'), amount: 22222 },
						{ mosaicId: generateMosaicAliasId('symbol.xym'), amount: 12345_000000n },
						{ mosaicId: generateMosaicAliasId('foo.bar'), amount: 1000 }
					]
				});

				const parser = new OperationParser(facade.network, { lookupCurrency: lookupCurrencyDefault });

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, transaction);

				// Assert:
				expect(operations).to.deep.equal([
					createTransferOperation(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '-22222', 'baz.bar', 1),
					createTransferOperation(1, 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ', '22222', 'baz.bar', 1),
					createTransferOperation(2, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '-12345000000', 'symbol.xym', 6),
					createTransferOperation(3, 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ', '12345000000', 'symbol.xym', 6),
					createTransferOperation(4, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '-1000', 'foo.bar', 3),
					createTransferOperation(5, 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ', '1000', 'foo.bar', 3)
				]);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal(['TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI']);
			});

			it('can parse with multiple mosaic and filter out zero transfers', async () => {
				// Arrange:
				const facade = new SymbolFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type: 'transfer_transaction_v1',
					recipientAddress: 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ',
					signerPublicKey: '527068DA90B142D98D27FF9BA2103A54230E3C8FAC8529E804123D986CACDCC9',
					mosaics: [
						{ mosaicId: generateMosaicAliasId('baz.bar'), amount: 22222 },
						{ mosaicId: generateMosaicAliasId('symbol.xym'), amount: 0 },
						{ mosaicId: generateMosaicAliasId('foo.bar'), amount: 1000 }
					]
				});

				const parser = new OperationParser(facade.network, { lookupCurrency: lookupCurrencyDefault });

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
			const assertCanParse = async propertyName => {
				// Arrange:
				const metadata = {
					minApprovalDelta: 1,
					minRemovalDelta: 2,
					[propertyName]: ['TCIO5J4WTXCVC76XZPBWHHNAD2NU52U2MOOVN4Q', 'TBO3SFA2XM3HY5QPBT7C6CCY4K6VXTHIQGPG6OQ']
				};

				const facade = new SymbolFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type: 'multisig_account_modification_transaction_v1',
					signerPublicKey: '527068DA90B142D98D27FF9BA2103A54230E3C8FAC8529E804123D986CACDCC9',
					...metadata
				});

				const parser = new OperationParser(facade.network);

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, transaction);

				// Assert:
				expect(operations).to.deep.equal([
					createMultisigOperation(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', metadata)
				]);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal(['TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI']);
			};

			it('can parse additions', () => assertCanParse('addressAdditions'));

			it('can parse deletions', () => assertCanParse('addressDeletions'));
		});

		// endregion

		// region supply change

		describe('supply change', () => {
			const assertCanParse = async (action, expectedAmount) => {
				// Arrange:
				const facade = new SymbolFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type: 'mosaic_supply_change_transaction_v1',
					signerPublicKey: '527068DA90B142D98D27FF9BA2103A54230E3C8FAC8529E804123D986CACDCC9',
					mosaicId: generateMosaicAliasId('foo.bar'),
					action,
					delta: 24680n
				});

				const parser = new OperationParser(facade.network, { lookupCurrency: lookupCurrencyDefault });

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, transaction);

				// Assert:
				expect(operations).to.deep.equal([
					createTransferOperation(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', expectedAmount, 'foo.bar', 3)
				]);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal(['TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI']);
			};

			it('can parse increase', () => assertCanParse(models.MosaicSupplyChangeAction.INCREASE, '24680'));

			it('can parse decrease', () => assertCanParse(models.MosaicSupplyChangeAction.DECREASE, '-24680'));
		});

		// endregion

		// region supply revocation

		describe('supply revocation', () => {
			it('can parse', async () => {
				// Arrange:
				const facade = new SymbolFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type: 'mosaic_supply_revocation_transaction_v1',
					signerPublicKey: '527068DA90B142D98D27FF9BA2103A54230E3C8FAC8529E804123D986CACDCC9',
					sourceAddress: 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ',
					mosaic: {
						mosaicId: generateMosaicAliasId('foo.bar'),
						amount: 24680n
					}
				});

				const parser = new OperationParser(facade.network, { lookupCurrency: lookupCurrencyDefault });

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, transaction);

				// Assert:
				expect(operations).to.deep.equal([
					createTransferOperation(0, 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ', '-24680', 'foo.bar', 3),
					createTransferOperation(1, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '24680', 'foo.bar', 3)
				]);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal(['TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI']);
			});
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
					divisibility: 2
				});

				const parser = new OperationParser(facade.network, {
					...options,
					lookupCurrency: lookupCurrencyDefault
				});

				// Act: add size and maxFee to JSON to emulate REST JSON
				const { operations, signerAddresses } = await parser.parseTransaction(
					{ ...convertTransactionSdkJsonToRestJson(transaction.toJson()), size: 100, maxFee: 123456 },
					metadata
				);

				// Assert:
				expect(operations).to.deep.equal(expectedOperations);
				expect(signerAddresses.map(address => address.toString())).to.deep.equal(['TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI']);
			};

			it('can parse', async () => assertCanParse({}, {}, []));

			it('can parse with max fee (unconfirmed)', async () => assertCanParse({ includeFeeOperation: true }, {}, [
				createTransferOperation(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '-123456', 'currency.fee', 2)
			]));

			it('can parse with fee (confirmed)', async () => assertCanParse({ includeFeeOperation: true }, { feeMultiplier: 200 }, [
				createTransferOperation(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '-20000', 'currency.fee', 2)
			]));
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
					addressAdditions: ['TCIO5J4WTXCVC76XZPBWHHNAD2NU52U2MOOVN4Q', 'TBO3SFA2XM3HY5QPBT7C6CCY4K6VXTHIQGPG6OQ']
				});
				verifier.addTransfer(
					'93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690D7',
					'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI',
					50n,
					'foo.bar'
				);
				verifier.addTransfer(
					'ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6',
					'TCJEJJBKDI62U4ZMO4VI7YAUVJE4STVCOBDSHXQ',
					33n
				);

				verifier.buildAggregate(
					'ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6',
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
					addressAdditions: ['TCIO5J4WTXCVC76XZPBWHHNAD2NU52U2MOOVN4Q', 'TBO3SFA2XM3HY5QPBT7C6CCY4K6VXTHIQGPG6OQ']
				}),
				createTransferOperation(3, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', '-50', 'foo.bar', 3),
				createTransferOperation(4, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '50', 'foo.bar', 3),
				createTransferOperation(5, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '-33', 'symbol.xym', 6),
				createTransferOperation(6, 'TCJEJJBKDI62U4ZMO4VI7YAUVJE4STVCOBDSHXQ', '33', 'symbol.xym', 6)
			]);

			it('can parse multiple transactions', async () => {
				// Arrange:
				const verifier = createDefaultVerifier(1);

				const parser = new OperationParser(verifier.facade.network, { lookupCurrency: lookupCurrencyDefault });

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, verifier.aggregateTransaction);

				// Assert:
				expect(operations).to.deep.equal(createDefaultVerifierOperations());
				expect(signerAddresses.map(address => address.toString()).sort()).to.deep.equal([
					'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI',
					'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ',
					'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI'
				]);
			});

			it('can parse multiple transactions with explicit cosigners', async () => {
				// Arrange:
				const verifier = createDefaultVerifier(2);

				verifier.addCosignature('3119DA3BFF57385BB6F051B8A454F219CE519D28E50D5653F5F457486E9E8623');
				verifier.addCosignature('93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690D7'); // redundant

				const parser = new OperationParser(verifier.facade.network, { lookupCurrency: lookupCurrencyDefault });

				// Act:
				const { operations, signerAddresses } = await parseTransaction(parser, verifier.aggregateTransaction);

				// Assert:
				expect(operations).to.deep.equal([
					...createDefaultVerifierOperations(),
					createCosignOperation(7, 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ')
				]);
				expect(signerAddresses.map(address => address.toString()).sort()).to.deep.equal([
					'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI',
					'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ',
					'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ',
					'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI'
				]);
			});

			const assertCanParseWithFee = async (metadata, expectedFeeAmount) => {
				// Arrange:
				const verifier = createDefaultVerifier(2);

				verifier.addCosignature('3119DA3BFF57385BB6F051B8A454F219CE519D28E50D5653F5F457486E9E8623');
				verifier.addCosignature('93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690D7'); // redundant

				const parser = new OperationParser(verifier.facade.network, {
					includeFeeOperation: true,
					lookupCurrency: lookupCurrencyDefault
				});

				// Act: add size and maxFee to JSON to emulate REST JSON
				const { operations, signerAddresses } = await parser.parseTransaction({
					...convertTransactionSdkJsonToRestJson(verifier.aggregateTransaction.toJson()),
					size: 100,
					maxFee: 123456
				}, metadata);

				// Assert:
				expect(operations).to.deep.equal([
					...createDefaultVerifierOperations(),
					createCosignOperation(7, 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ'),
					createTransferOperation(8, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', expectedFeeAmount, 'currency.fee', 2)
				]);
				expect(signerAddresses.map(address => address.toString()).sort()).to.deep.equal([
					'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI',
					'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ',
					'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ',
					'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI'
				]);
			};

			it('can parse multiple transactions with explicit cosigners including max fee (unconfirmed)', () =>
				assertCanParseWithFee({}, '-123456'));

			it('can parse multiple transactions with explicit cosigners including fee (confirmed)', () =>
				assertCanParseWithFee({ feeMultiplier: 200 }, '-20000'));
		});

		// endregion

		// region location

		describe('location', () => {
			const runTopLevelLocationTest = async (metadata, expectedCurrencyName) => {
				// Arrange:
				const facade = new SymbolFacade('testnet');
				const transaction = facade.transactionFactory.create({
					type: 'transfer_transaction_v1',
					recipientAddress: 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ',
					signerPublicKey: '527068DA90B142D98D27FF9BA2103A54230E3C8FAC8529E804123D986CACDCC9',
					mosaics: [
						{ mosaicId: generateMosaicAliasId('check.location'), amount: 1000n }
					]
				});

				const parser = new OperationParser(facade.network, { lookupCurrency: lookupCurrencyDefault });

				// Act:
				const { operations } = await parseTransaction(parser, transaction, metadata);

				// Assert:
				expect(operations).to.deep.equal([
					createTransferOperation(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '-1000', expectedCurrencyName, 4),
					createTransferOperation(1, 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ', '1000', expectedCurrencyName, 4)
				]);
			};

			it('can pass down for top level transaction (valid)', () => runTopLevelLocationTest(
				{ height: '1234', index: 11 },
				'1234.12.0'
			));

			it('can pass down for top level transaction (height - undefined)', () => runTopLevelLocationTest(
				{ index: 11 },
				'undefined'
			));

			it('can pass down for top level transaction (height - zero)', () => runTopLevelLocationTest(
				{ height: '0', index: 11 },
				'undefined'
			));

			const runEmbeddedLocationTest = async (topLevelMetadata, subTransactionMetadatas, expectedCurrencyNames) => {
				// Arrange:
				const verifier = new PayloadResultVerifier();
				verifier.addTransfer(
					'ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6',
					'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI',
					100n,
					'check.location'
				);
				verifier.addTransfer(
					'93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690D7',
					'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI',
					50n,
					'check.location'
				);
				verifier.addTransfer(
					'ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6',
					'TCJEJJBKDI62U4ZMO4VI7YAUVJE4STVCOBDSHXQ',
					33n,
					'check.location'
				);

				verifier.buildAggregate(
					'ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6',
					1001n + (60n * 60n * 1000n),
					1
				);

				const aggregateTransactionJson = convertTransactionSdkJsonToRestJson(verifier.aggregateTransaction.toJson());
				aggregateTransactionJson.transactions.forEach((transaction, i) => {
					transaction.meta = subTransactionMetadatas[i];
				});

				const parser = new OperationParser(verifier.facade.network, { lookupCurrency: lookupCurrencyDefault });

				// Act:
				const { operations } = await parser.parseTransaction(aggregateTransactionJson, topLevelMetadata);

				// Assert:
				expect(operations).to.deep.equal([
					createTransferOperation(0, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '-100', expectedCurrencyNames[0], 4),
					createTransferOperation(1, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '100', expectedCurrencyNames[0], 4),
					createTransferOperation(2, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', '-50', expectedCurrencyNames[1], 4),
					createTransferOperation(3, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '50', expectedCurrencyNames[1], 4),
					createTransferOperation(4, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '-33', expectedCurrencyNames[2], 4),
					createTransferOperation(5, 'TCJEJJBKDI62U4ZMO4VI7YAUVJE4STVCOBDSHXQ', '33', expectedCurrencyNames[2], 4)
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
					'1234.12.5',
					'1234.12.8',
					'1234.12.4'
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
					'undefined',
					'undefined',
					'undefined'
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
					'undefined',
					'1234.12.8',
					'undefined'
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
				recipientAddress: 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ',
				signerPublicKey: '527068DA90B142D98D27FF9BA2103A54230E3C8FAC8529E804123D986CACDCC9',
				mosaics: [
					{ mosaicId: generateMosaicAliasId('symbol.xym'), amount: 12345_000000n }
				]
			});

			const parser = new OperationParser(facade.network, { lookupCurrency: lookupCurrencyDefault });

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

			const parser = new OperationParser(facade.network, { lookupCurrency: lookupCurrencyDefault });

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
			it('can parse', () => runReceiptTest({
				type: 1111,
				targetAddress: '982390440A195B82D4A06810038B1400CE7CDA7AB3F48F99',
				mosaicId: generateMosaicAliasId('foo.bar'),
				amount: '4741734'
			}, [
			]));
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
	});

	// endregion
});

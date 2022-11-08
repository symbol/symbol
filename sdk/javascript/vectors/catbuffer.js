import SymbolBlockFactory from './BlockFactory.js';
import ReceiptFactory from './ReceiptFactory.js';
import NemFacade from '../src/facade/NemFacade.js';
import SymbolFacade from '../src/facade/SymbolFacade.js';
import * as nc from '../src/nem/models.js';
import * as sc from '../src/symbol/models.js';
import * as converter from '../src/utils/converter.js';
import { expect } from 'chai';
import JSONBigIntLib from 'json-bigint';
import fs from 'fs';
import path from 'path';

const JSONBigInt = JSONBigIntLib({ alwaysParseAsBig: true, useNativeBigInt: true });

describe('catbuffer vectors', () => {
	// region common test utils

	const prepareTestCases = (networkName, options) => {
		const cases = [];
		const schemasPath = path.join(process.env.SCHEMAS_PATH || '.', networkName, 'models');

		if (!fs.existsSync(schemasPath))
			throw Error(`could not find any cases because ${schemasPath} does not exist`);

		fs.readdirSync(schemasPath).filter(name => name.endsWith('.json')).forEach(name => {
			if (options && options.includes && options.includes.every(include => !name.includes(include))) {
				console.log(`skipping ${name} due to include filters`);
				return;
			}

			if (options && options.excludes && options.excludes.some(exclude => name.includes(exclude))) {
				console.log(`skipping ${name} due to exclude filters`);
				return;
			}

			const fileContent = fs.readFileSync(path.join(schemasPath, name), 'utf8');
			cases.push(...JSONBigInt.parse(fileContent));
		});

		if (0 === cases.length)
			throw Error(`could not find any cases in ${schemasPath}`);

		return cases;
	};

	const makeCamelCase = name => {
		let camelCaseName = '';
		let lastCharWasUnderscore = false;
		Array.from(name).forEach(ch => {
			if ('_' === ch) {
				lastCharWasUnderscore = true;
				return;
			}

			camelCaseName += lastCharWasUnderscore ? ch.toUpperCase() : ch;
			lastCharWasUnderscore = false;
		});

		return camelCaseName;
	};

	const isNumericNem = (key, value, type) => {
		const bigIntPropertyNames = [
			'amount', 'fee', 'rental_fee'
		];

		if (bigIntPropertyNames.some(name => key === name))
			return false;

		if ('delta' === key && 'mosaic_supply_change_transaction_v1' === type)
			return false;

		return 0xFFFFFFFFn >= value;
	};

	const isNumericSymbol = (key, value, type) => {
		const bigIntPropertyNames = [
			'amount', 'fee', 'duration', 'scoped_metadata_key', 'restriction_key',
			'new_restriction_value', 'previous_restriction_value',
			'mosaic_id', 'reference_mosaic_id', 'target_mosaic_id',
			'namespace_id', 'target_namespace_id',

			'height', 'difficulty', 'timestamp', 'harvesting_eligible_accounts_count'
		];

		if (bigIntPropertyNames.some(name => key === name))
			return false;

		if ('delta' === key && 'mosaic_supply_change_transaction_v1' === type)
			return false;

		return 0xFFFFFFFFn >= value;
	};

	const jsifyImpl = isNumeric => source => {
		const dest = {};
		Object.getOwnPropertyNames(source).forEach(key => {
			let value = source[key];

			if ('bigint' === typeof (value) && isNumeric(key, value, source.type)) {
				value = Number(value);
			} else if (Array.isArray(value)) {
				value = value.map(valueItem => {
					if ('bigint' === typeof (valueItem))
						return 'account_mosaic_restriction_transaction_v1' === source.type ? valueItem : Number(valueItem);

					if ('object' === typeof (valueItem))
						return jsifyImpl(isNumeric)(valueItem);

					return valueItem;
				});
			} else if ('object' === typeof (value) && null !== value) {
				value = jsifyImpl(isNumeric)(value);
			}

			dest[makeCamelCase(key)] = value;
		});

		return dest;
	};

	const jsifySymbol = jsifyImpl(isNumericSymbol);

	const jsifyNem = jsifyImpl(isNumericNem);

	// endregion

	// region create from descriptor

	describe('create from descriptor', () => {
		const fixupDescriptorCommon = (descriptor, module) => {
			Object.getOwnPropertyNames(descriptor).forEach(key => {
				// skip false positive due to ABC123 value that should be treated as plain string
				if ('value' === key && 'namespace_metadata_transaction_v1' === descriptor.type)
					return;

				const value = descriptor[key];
				if ('string' === typeof (value) && converter.isHexString(value))
					descriptor[key] = converter.hexToUint8(value);
				else if ('object' === typeof (value) && null !== value)
					fixupDescriptorCommon(value, module);
			});
		};

		const fixupDescriptorNem = (descriptor, module, facade) => {
			descriptor.signature = new module.Signature(descriptor.signature);
			fixupDescriptorCommon(descriptor, module);

			if (descriptor.innerTransaction) {
				fixupDescriptorNem(descriptor.innerTransaction, module, facade);
				descriptor.innerTransaction = facade.transactionFactory.constructor
					.toNonVerifiableTransaction(facade.transactionFactory.create(descriptor.innerTransaction));

				if (descriptor.cosignatures) {
					descriptor.cosignatures = descriptor.cosignatures.map(cosignatureDescriptor => {
						const cosignature = facade.transactionFactory.create({
							type: 'cosignature_v1',
							...cosignatureDescriptor.cosignature
						});
						cosignature.signature = new module.Signature(cosignatureDescriptor.cosignature.signature);
						cosignature.network = module.NetworkType.MAINNET; // TODO: fixup based on mismatch in vectors

						const sizePrefixedCosignature = new module.SizePrefixedCosignatureV1();
						sizePrefixedCosignature.cosignature = cosignature;
						return sizePrefixedCosignature;
					});
				}
			}
		};

		const fixupCosignatureSymbol = (descriptor, module) => {
			const cosignature = new module.Cosignature();
			cosignature.signature = new module.Signature(descriptor.signature);
			cosignature.signerPublicKey = new module.PublicKey(descriptor.signerPublicKey);
			return cosignature;
		};

		const fixupDescriptorSymbol = (descriptor, module, facade) => {
			descriptor.signature = new module.Signature(descriptor.signature);
			fixupDescriptorCommon(descriptor, module);

			if (descriptor.transactions) {
				descriptor.transactions = descriptor.transactions
					.map(childDescriptor => facade.transactionFactory.createEmbedded(childDescriptor));

				if (descriptor.cosignatures) {
					descriptor.cosignatures = descriptor.cosignatures
						.map(cosignatureDescriptor => fixupCosignatureSymbol(cosignatureDescriptor, module));
				}
			}

			return descriptor;
		};

		const fixupBlockDescriptorSymbol = (descriptor, module, facade) => {
			descriptor.signature = new module.Signature(descriptor.signature);
			fixupDescriptorCommon(descriptor, module);

			if (descriptor.transactions) {
				descriptor.transactions = descriptor.transactions
					.map(childDescriptor => fixupDescriptorSymbol(childDescriptor, module, facade));

				descriptor.transactions = descriptor.transactions
					.map(childDescriptor => facade.transactionFactory.create(childDescriptor));
			}
		};

		const isKeyInFormattedString = (transaction, key) => {
			if (transaction.toString().includes(key))
				return true;

			return 'parentName' === key && null === transaction[key];
		};

		const assertCreateFromDescriptor = (item, module, FacadeClass, fixupDescriptor, jsify) => {
			// Arrange:
			const facade = new FacadeClass('testnet');

			const descriptor = jsify(item.descriptor);
			fixupDescriptor(descriptor, module, facade);

			// Act:
			const transaction = facade.transactionFactory.create(descriptor);
			const transactionBuffer = transaction.serialize();

			// Assert:
			expect(converter.uint8ToHex(transactionBuffer)).to.equal(item.payload);
			expect(Object.getOwnPropertyNames(descriptor).every(key => isKeyInFormattedString(transaction, key)))
				.to.equal(true);
		};

		const createSymbolDescriptor = (originalDescriptor, fixupDescriptor) => {
			const facade = new SymbolFacade('testnet');

			// - this will be dealing with symbol blocks only
			const descriptor = jsifySymbol(originalDescriptor);
			fixupDescriptor(descriptor, sc, facade);

			return { network: facade.network, descriptor };
		};

		const assertCreateSymbolBlockFromDescriptor = (item, fixupDescriptor) => {
			// Arrange:
			const { network, descriptor } = createSymbolDescriptor(item.descriptor, fixupDescriptor);

			// Act:
			const blockFactory = new SymbolBlockFactory(network);
			const block = blockFactory.create(descriptor);
			const blockBuffer = block.serialize();

			// Assert:
			expect(converter.uint8ToHex(blockBuffer)).to.equal(item.payload);
			expect(Object.getOwnPropertyNames(descriptor).every(key => isKeyInFormattedString(block, key)))
				.to.equal(true);
		};

		const assertCreateSymbolReceiptFromDescriptor = (item, fixupDescriptor) => {
			// Arrange:
			const { descriptor } = createSymbolDescriptor(item.descriptor, fixupDescriptor);

			// Act:
			const receiptFactory = new ReceiptFactory();
			const receipt = receiptFactory.create(descriptor);
			const receiptBuffer = receipt.serialize();

			// Assert:
			expect(converter.uint8ToHex(receiptBuffer)).to.equal(item.payload);
			expect(Object.getOwnPropertyNames(descriptor).every(key => isKeyInFormattedString(receipt, key)))
				.to.equal(true);
		};

		describe('NEM', () => {
			prepareTestCases('nem').forEach(item => {
				it(`can create from descriptor ${item.test_name}`, () => {
					assertCreateFromDescriptor(item, nc, NemFacade, fixupDescriptorNem, jsifyNem);
				});
			});
		});

		describe('Symbol (transactions)', () => {
			prepareTestCases('symbol', { includes: ['transactions'] }).forEach(item => {
				it(`can create from descriptor ${item.test_name}`, () => {
					assertCreateFromDescriptor(item, sc, SymbolFacade, fixupDescriptorSymbol, jsifySymbol);
				});
			});
		});

		describe('Symbol (blocks)', () => {
			prepareTestCases('symbol', { includes: ['blocks'] }).forEach(item => {
				it(`can create from descriptor ${item.test_name}`, () => {
					assertCreateSymbolBlockFromDescriptor(item, fixupBlockDescriptorSymbol);
				});
			});
		});

		describe('Symbol (receipts)', () => {
			prepareTestCases('symbol', { includes: ['receipts'] }).forEach(item => {
				it(`can create from descriptor ${item.test_name}`, () => {
					assertCreateSymbolReceiptFromDescriptor(item, () => {});
				});
			});
		});
	});

	// endregion

	// region create from constructor

	describe('create from constructor', () => {
		const assertCreateFromConstructor = (schemaName, module) => {
			// Arrange:
			const SchemaClass = module[schemaName];

			// Act:
			const transaction = new SchemaClass();

			const { size } = transaction;
			const transactionBuffer = transaction.serialize();

			// Assert:
			expect(size).to.not.equal(0);
			expect(transactionBuffer.length).to.not.equal(0);
			expect(size).to.equal(transactionBuffer.length);
		};

		describe('NEM', () => {
			new Set(prepareTestCases('nem').map(item => item.schema_name)).forEach(schemaName => {
				it(`can create from constructor ${schemaName}`, () => {
					assertCreateFromConstructor(schemaName, nc);
				});
			});
		});

		describe('Symbol', () => {
			new Set(prepareTestCases('symbol').map(item => item.schema_name)).forEach(schemaName => {
				it(`can create from constructor ${schemaName}`, () => {
					assertCreateFromConstructor(schemaName, sc);
				});
			});
		});
	});

	// endregion

	// region roundtrip

	describe('roundtrip', () => {
		const assertRoundtrip = (item, module) => {
			// Arrange:
			const schemaName = item.schema_name;
			const payloadHex = item.payload;
			const payload = converter.hexToUint8(payloadHex);

			const TransactionClass = module[schemaName];

			// Act:
			const transaction = TransactionClass.deserialize(payload);
			const transactionBuffer = transaction.serialize();

			// Assert:
			expect(converter.uint8ToHex(transactionBuffer)).to.equal(payloadHex);
			expect(transaction.size).to.equal(transactionBuffer.length);

			if (schemaName.endsWith('Transaction'))
				assertRoundtrip({ schema_name: 'TransactionFactory', payload: payloadHex }, module);
		};

		describe('NEM', () => {
			prepareTestCases('nem').forEach(item => {
				it(`can roundtrip ${item.test_name}`, () => {
					assertRoundtrip(item, nc);
				});
			});
		});

		describe('Symbol', () => {
			prepareTestCases('symbol').forEach(item => {
				it(`can roundtrip ${item.test_name}`, () => {
					assertRoundtrip(item, sc);
				});
			});
		});
	});

	// endregion
});

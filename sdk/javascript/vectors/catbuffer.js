const { NemFacade } = require('../src/facade/NemFacade');
const { SymbolFacade } = require('../src/facade/SymbolFacade');
const nc = require('../src/nem/models');
const sc = require('../src/symbol/models');
const converter = require('../src/utils/converter');
const { expect } = require('chai');
const JSONBigInt = require('json-bigint')({ alwaysParseAsBig: true, useNativeBigInt: true });
const fs = require('fs');
const path = require('path');

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

	const isNumeric = (key, value, type) => {
		const bigIntPropertyNames = [
			'amount', 'fee', 'mosaic_id', 'duration', 'scoped_metadata_key', 'namespace_id', 'restriction_key', 'restriction_value'
		];

		if (bigIntPropertyNames.some(name => key.includes(name)))
			return false;

		if ('delta' === key && 'mosaic_supply_change_transaction' === type)
			return false;

		return 0xFFFFFFFFn >= value;
	};

	const jsify = source => {
		const dest = {};
		Object.getOwnPropertyNames(source).forEach(key => {
			let value = source[key];

			if ('bigint' === typeof (value) && isNumeric(key, value, source.type)) {
				value = Number(value);
			} else if (Array.isArray(value)) {
				value = value.map(valueItem => {
					if ('bigint' === typeof (valueItem))
						return 'account_mosaic_restriction_transaction' === source.type ? valueItem : Number(valueItem);

					if ('object' === typeof (valueItem))
						return jsify(valueItem);

					return valueItem;
				});
			} else if ('object' === typeof (value) && null !== value) {
				value = jsify(value);
			}

			dest[makeCamelCase(key)] = value;
		});

		return dest;
	};

	// endregion

	// region create from descriptor

	describe('create from descriptor', () => {
		const fixupDescriptorCommon = (descriptor, module) => {
			Object.getOwnPropertyNames(descriptor).forEach(key => {
				// skip false positive due to ABC123 value that should be treated as plain string
				if ('value' === key && 'namespace_metadata_transaction' === descriptor.type)
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
							type: 'cosignature',
							...cosignatureDescriptor.cosignature
						});
						cosignature.signature = new module.Signature(cosignatureDescriptor.cosignature.signature);
						cosignature.network = module.NetworkType.MAINNET; // TODO: fixup based on mismatch in vectors

						const sizePrefixedCosignature = new module.SizePrefixedCosignature();
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
		};

		const isKeyInFormattedString = (transaction, key) => {
			if (transaction.toString().includes(key))
				return true;

			return 'parentName' === key && null === transaction[key];
		};

		const assertCreateFromDescriptor = (item, module, FacadeClass, fixupDescriptor) => {
			// Arrange:
			const comment = item.comment || '';
			const payloadHex = item.payload;

			const facade = new FacadeClass('testnet');

			const descriptor = jsify(item.descriptor);
			fixupDescriptor(descriptor, module, facade);

			// Act:
			const transaction = facade.transactionFactory.create(descriptor);
			const transactionBuffer = transaction.serialize();

			// Assert:
			expect(converter.uint8ToHex(transactionBuffer), comment).to.equal(payloadHex);
			expect(Object.getOwnPropertyNames(descriptor).every(key => isKeyInFormattedString(transaction, key), comment))
				.to.equal(true);
		};

		describe('NEM', () => {
			prepareTestCases('nem').forEach(item => {
				it(`can create from descriptor ${item.test_name}`, () => {
					assertCreateFromDescriptor(item, nc, NemFacade, fixupDescriptorNem);
				});
			});
		});

		describe('Symbol', () => {
			prepareTestCases('symbol').forEach(item => {
				it(`can create from descriptor ${item.test_name}`, () => {
					assertCreateFromDescriptor(item, sc, SymbolFacade, fixupDescriptorSymbol);
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
			const comment = item.comment || '';
			const payloadHex = item.payload;
			const payload = converter.hexToUint8(payloadHex);

			const TransactionClass = module[schemaName];

			// Act:
			const transaction = TransactionClass.deserialize(payload);
			const transactionBuffer = transaction.serialize();

			// Assert:
			expect(converter.uint8ToHex(transactionBuffer), comment).to.equal(payloadHex);
			expect(transaction.size, comment).to.equal(transactionBuffer.length);

			if (schemaName.endsWith('Transaction'))
				assertRoundtrip({ schema_name: 'TransactionFactory', payload: payloadHex, comment }, module);
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

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

import { OperationParser, convertTransactionSdkJsonToRestJson } from './OperationParser.js';
import { createLookupCurrencyFunction, getBlockchainDescriptor } from './rosettaUtils.js';
import AccountIdentifier from '../openApi/model/AccountIdentifier.js';
import ConstructionCombineRequest from '../openApi/model/ConstructionCombineRequest.js';
import ConstructionCombineResponse from '../openApi/model/ConstructionCombineResponse.js';
import ConstructionDeriveRequest from '../openApi/model/ConstructionDeriveRequest.js';
import ConstructionDeriveResponse from '../openApi/model/ConstructionDeriveResponse.js';
import ConstructionHashRequest from '../openApi/model/ConstructionHashRequest.js';
import ConstructionMetadataRequest from '../openApi/model/ConstructionMetadataRequest.js';
import ConstructionMetadataResponse from '../openApi/model/ConstructionMetadataResponse.js';
import ConstructionParseRequest from '../openApi/model/ConstructionParseRequest.js';
import ConstructionParseResponse from '../openApi/model/ConstructionParseResponse.js';
import ConstructionPayloadsRequest from '../openApi/model/ConstructionPayloadsRequest.js';
import ConstructionPayloadsResponse from '../openApi/model/ConstructionPayloadsResponse.js';
import ConstructionPreprocessRequest from '../openApi/model/ConstructionPreprocessRequest.js';
import ConstructionPreprocessResponse from '../openApi/model/ConstructionPreprocessResponse.js';
import ConstructionSubmitRequest from '../openApi/model/ConstructionSubmitRequest.js';
import CurveType from '../openApi/model/CurveType.js';
import SignatureType from '../openApi/model/SignatureType.js';
import SigningPayload from '../openApi/model/SigningPayload.js';
import TransactionIdentifier from '../openApi/model/TransactionIdentifier.js';
import TransactionIdentifierResponse from '../openApi/model/TransactionIdentifierResponse.js';
import {
	RosettaErrorFactory, RosettaPublicKeyProcessor, extractTransferDescriptorAt, rosettaPostRouteWithNetwork
} from '../rosettaUtils.js';
import { PrivateKey, PublicKey, utils } from 'symbol-sdk';
import {
	KeyPair, NetworkTimestamp, SymbolFacade, generateMosaicAliasId, models
} from 'symbol-sdk/symbol';

export default {
	register: (server, db, services) => {
		const blockchainDescriptor = getBlockchainDescriptor(services.config);
		const facade = new SymbolFacade(blockchainDescriptor.network);
		const lookupCurrency = createLookupCurrencyFunction(services.proxy);
		const publicKeyProcessor = new RosettaPublicKeyProcessor(new CurveType().edwards25519, facade.network, PublicKey);

		const aggregateSignerKeyPair = new KeyPair(new PrivateKey(services.config.rosetta.aggregateSignerPrivateKey));
		const aggregateSignerAddress = facade.network.publicKeyToAddress(aggregateSignerKeyPair.publicKey);

		const constructionPostRoute = (...args) => rosettaPostRouteWithNetwork(blockchainDescriptor, ...args);

		server.post('/construction/derive', constructionPostRoute(ConstructionDeriveRequest, typedRequest => {
			const publicKey = publicKeyProcessor.parsePublicKey(typedRequest.public_key);
			const address = facade.network.publicKeyToAddress(publicKey);

			const response = new ConstructionDeriveResponse();
			response.account_identifier = new AccountIdentifier(address.toString());
			return response;
		}));

		server.post('/construction/preprocess', constructionPostRoute(ConstructionPreprocessRequest, typedRequest => {
			const response = new ConstructionPreprocessResponse();
			response.options = {};
			response.required_public_keys = [];

			typedRequest.operations.forEach(operation => {
				if ('transfer' === operation.type) {
					if (0n > BigInt(operation.amount.value))
						response.required_public_keys.push(operation.account);
				} else if (['multisig', 'cosign'].includes(operation.type)) {
					response.required_public_keys.push(operation.account);
				}
			});

			return response;
		}));

		const getNetworkTime = () => services.proxy.fetch('node/time', jsonObject => jsonObject.communicationTimestamps.receiveTimestamp);
		const getSuggestedTransactionMultiplier = () => services.proxy.fetch(
			'network/fees/transaction',
			jsonObject => jsonObject.averageFeeMultiplier
		);

		server.post('/construction/metadata', constructionPostRoute(ConstructionMetadataRequest, async () => {
			// ignore request object because only global metadata is needed for transaction construction

			const results = await Promise.all([getNetworkTime(), getSuggestedTransactionMultiplier()]);

			const response = new ConstructionMetadataResponse();
			response.metadata = {
				networkTime: results[0],
				feeMultiplier: results[1]
			};
			return response;
		}));

		const createSigningPayload = (signingPayload, signerAddress) => {
			const result = new SigningPayload(utils.uint8ToHex(signingPayload));
			result.account_identifier = new AccountIdentifier(signerAddress.toString());
			result.signature_type = new SignatureType().ed25519;
			return result;
		};

		const toPublicKeySet = publicKeys => {
			const publicKeyStringSet = new Set(publicKeys.map(publicKey => publicKey.toString()));
			return [...publicKeyStringSet].sort().map(publicKeyString => new models.PublicKey(publicKeyString));
		};

		server.post('/construction/payloads', constructionPostRoute(ConstructionPayloadsRequest, async typedRequest => {
			const addressToPublicKeyMap = publicKeyProcessor.buildAddressToPublicKeyMap(typedRequest.public_keys);

			let hasMultisigModification = false;
			const embeddedTransactions = [];
			const allSignerPublicKeys = [];
			for (let i = 0; i < typedRequest.operations.length; ++i) {
				const operation = typedRequest.operations[i];

				if ('transfer' === operation.type) {
					const transferDescriptor = extractTransferDescriptorAt(typedRequest.operations, i);
					const embeddedTransaction = facade.transactionFactory.createEmbedded({
						type: 'transfer_transaction_v1',
						signerPublicKey: addressToPublicKeyMap[transferDescriptor.senderAddress],
						recipientAddress: transferDescriptor.recipientAddress,
						mosaics: [
							{ mosaicId: generateMosaicAliasId('symbol.xym'), amount: transferDescriptor.amount }
						]
					});

					embeddedTransactions.push(embeddedTransaction);
					allSignerPublicKeys.push(embeddedTransaction.signerPublicKey);
					++i;
				} else if ('multisig' === operation.type) {
					const embeddedTransaction = facade.transactionFactory.createEmbedded({
						type: 'multisig_account_modification_transaction_v1',
						signerPublicKey: addressToPublicKeyMap[operation.account.address],
						...operation.metadata
					});

					embeddedTransactions.push(embeddedTransaction);
					hasMultisigModification = true;
				} else if ('cosign' === operation.type) {
					allSignerPublicKeys.push(new models.PublicKey(addressToPublicKeyMap[operation.account.address].bytes));
				}
			}

			let cosignerPublicKeys = toPublicKeySet(allSignerPublicKeys);
			const hasCosigners = hasMultisigModification || 1 < cosignerPublicKeys.length;

			let aggregateSignerPublicKey;
			if (!hasCosigners) {
				aggregateSignerPublicKey = new PublicKey(embeddedTransactions[0].signerPublicKey.bytes);
				cosignerPublicKeys = [];
			} else {
				aggregateSignerPublicKey = aggregateSignerKeyPair.publicKey;
				const firstEmbeddedTransactionPublicKey = new PublicKey(embeddedTransactions[0].signerPublicKey.bytes);

				// create embedded transaction that will require signing by fee payer
				embeddedTransactions.unshift(facade.transactionFactory.createEmbedded({
					type: 'transfer_transaction_v1',
					signerPublicKey: aggregateSignerKeyPair.publicKey,
					recipientAddress: facade.network.publicKeyToAddress(firstEmbeddedTransactionPublicKey),
					mosaics: [
						{ mosaicId: generateMosaicAliasId('symbol.xym'), amount: 0 }
					]
				}));

				// transfer max fee to fee payer (aggregate signer) with placeholder amount
				embeddedTransactions.unshift(facade.transactionFactory.createEmbedded({
					type: 'transfer_transaction_v1',
					signerPublicKey: firstEmbeddedTransactionPublicKey,
					recipientAddress: aggregateSignerAddress,
					mosaics: [
						{ mosaicId: generateMosaicAliasId('symbol.xym'), amount: 0 }
					]
				}));
			}

			const aggregateTransaction = facade.transactionFactory.create({
				type: 'aggregate_complete_transaction_v2',
				signerPublicKey: aggregateSignerPublicKey,
				deadline: new NetworkTimestamp(typedRequest.metadata.networkTime).addHours(1).timestamp,
				transactionsHash: facade.static.hashEmbeddedTransactions(embeddedTransactions),
				transactions: embeddedTransactions,

				cosignatures: cosignerPublicKeys.map(cosignerPublicKey => {
					const cosignature = new models.Cosignature();
					cosignature.signerPublicKey = cosignerPublicKey;
					return cosignature;
				})
			});

			const transactionSize = aggregateTransaction.size;
			aggregateTransaction.fee = new models.Amount(transactionSize * typedRequest.metadata.feeMultiplier);

			if (hasCosigners) {
				// transfer fee to fee payer (aggregate signer)
				embeddedTransactions[0].mosaics[0].amount.value = aggregateTransaction.fee.value;

				// update transactions hash
				const transactionsHash = facade.static.hashEmbeddedTransactions(embeddedTransactions);
				aggregateTransaction.transactionsHash = new models.Hash256(transactionsHash.bytes);

				// sign with fee payer
				const signature = facade.signTransaction(aggregateSignerKeyPair, aggregateTransaction);
				facade.transactionFactory.static.attachSignature(aggregateTransaction, signature);
			}

			const transactionHash = facade.hashTransaction(aggregateTransaction);
			const transactionPayload = aggregateTransaction.serialize();
			const signingPayload = facade.extractSigningPayload(aggregateTransaction);

			const response = new ConstructionPayloadsResponse();
			response.unsigned_transaction = utils.uint8ToHex(transactionPayload);

			response.payloads = !hasCosigners
				? [createSigningPayload(signingPayload, facade.network.publicKeyToAddress(aggregateTransaction.signerPublicKey))]
				: cosignerPublicKeys.map(publicKey => createSigningPayload(
					transactionHash.bytes,
					facade.network.publicKeyToAddress(publicKey)
				));

			return response;
		}));

		server.post('/construction/combine', constructionPostRoute(ConstructionCombineRequest, async typedRequest => {
			if (typedRequest.signatures.some(signature => new SignatureType().ed25519 !== signature.signature_type))
				throw RosettaErrorFactory.UNSUPPORTED_CURVE;

			const findSignature = publicKey => typedRequest.signatures.find(rosettaSignature =>
				0 === utils.deepCompare(publicKey.bytes, utils.hexToUint8(rosettaSignature.public_key.hex_bytes))).hex_bytes;

			const aggregateTransaction = facade.transactionFactory.static.deserialize(utils.hexToUint8(typedRequest.unsigned_transaction));

			if (!aggregateTransaction.cosignatures.length) {
				aggregateTransaction.signature = new models.Signature(findSignature(aggregateTransaction.signerPublicKey));
			} else {
				aggregateTransaction.cosignatures.forEach(cosignature => {
					cosignature.signature = new models.Signature(findSignature(cosignature.signerPublicKey));
				});
			}

			const transactionPayload = aggregateTransaction.serialize();

			const response = new ConstructionCombineResponse();
			response.signed_transaction = utils.uint8ToHex(transactionPayload);
			return response;
		}));

		server.post('/construction/parse', constructionPostRoute(ConstructionParseRequest, async typedRequest => {
			const currencyMosaicId = generateMosaicAliasId('symbol.xym');
			const xymCurrency = await lookupCurrency('currencyMosaicId'); // need to store resolved id in currency metadata

			const aggregateTransaction = facade.transactionFactory.static.deserialize(utils.hexToUint8(typedRequest.transaction));

			const parser = new OperationParser(facade.network, {
				lookupCurrency: mosaicId => {
					if (currencyMosaicId !== mosaicId)
						throw RosettaErrorFactory.NOT_SUPPORTED_ERROR;

					return xymCurrency;
				},
				resolveAddress: address => Promise.resolve(address)
			});

			const aggregateTransactionJson = convertTransactionSdkJsonToRestJson(aggregateTransaction.toJson());
			const { operations, signerAddresses } = await parser.parseTransaction(aggregateTransactionJson);

			const response = new ConstructionParseResponse();
			response.operations = operations;
			response.account_identifier_signers = [];
			response.signers = [];

			if (typedRequest.signed) {
				response.account_identifier_signers = signerAddresses
					.map(address => address.toString())
					.filter(addressString => aggregateSignerAddress.toString() !== addressString)
					.map(addressString => new AccountIdentifier(addressString));
			}

			return response;
		}));

		const processTransactionHashRequest = typedRequest => {
			const aggregateTransaction = facade.transactionFactory.static.deserialize(utils.hexToUint8(typedRequest.signed_transaction));
			const transactionHash = facade.hashTransaction(aggregateTransaction);

			const response = new TransactionIdentifierResponse();
			response.transaction_identifier = new TransactionIdentifier(transactionHash.toString());
			return response;
		};

		server.post('/construction/hash', constructionPostRoute(ConstructionHashRequest, async typedRequest =>
			processTransactionHashRequest(typedRequest)));

		server.post('/construction/submit', constructionPostRoute(ConstructionSubmitRequest, async typedRequest => {
			await services.proxy.fetch('transactions', jsonObject => jsonObject.message, {
				method: 'PUT',
				headers: { 'Content-Type': 'application/json' },
				body: JSON.stringify({ payload: typedRequest.signed_transaction })
			});

			return processTransactionHashRequest(typedRequest);
		}));
	}
};

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
import AccountIdentifier from './openApi/model/AccountIdentifier.js';
import ConstructionCombineRequest from './openApi/model/ConstructionCombineRequest.js';
import ConstructionCombineResponse from './openApi/model/ConstructionCombineResponse.js';
import ConstructionDeriveRequest from './openApi/model/ConstructionDeriveRequest.js';
import ConstructionDeriveResponse from './openApi/model/ConstructionDeriveResponse.js';
import ConstructionHashRequest from './openApi/model/ConstructionHashRequest.js';
import ConstructionMetadataRequest from './openApi/model/ConstructionMetadataRequest.js';
import ConstructionMetadataResponse from './openApi/model/ConstructionMetadataResponse.js';
import ConstructionParseRequest from './openApi/model/ConstructionParseRequest.js';
import ConstructionParseResponse from './openApi/model/ConstructionParseResponse.js';
import ConstructionPayloadsRequest from './openApi/model/ConstructionPayloadsRequest.js';
import ConstructionPayloadsResponse from './openApi/model/ConstructionPayloadsResponse.js';
import ConstructionPreprocessRequest from './openApi/model/ConstructionPreprocessRequest.js';
import ConstructionPreprocessResponse from './openApi/model/ConstructionPreprocessResponse.js';
import ConstructionSubmitRequest from './openApi/model/ConstructionSubmitRequest.js';
import CurveType from './openApi/model/CurveType.js';
import SignatureType from './openApi/model/SignatureType.js';
import SigningPayload from './openApi/model/SigningPayload.js';
import TransactionIdentifier from './openApi/model/TransactionIdentifier.js';
import TransactionIdentifierResponse from './openApi/model/TransactionIdentifierResponse.js';
import { RosettaErrorFactory, createLookupCurrencyFunction, rosettaPostRouteWithNetwork } from './rosettaUtils.js';
import { NetworkLocator, PublicKey, utils } from 'symbol-sdk';
import {
	Network, NetworkTimestamp, SymbolFacade, generateMosaicAliasId, models
} from 'symbol-sdk/symbol';

export default {
	register: (server, db, services) => {
		const networkName = services.config.network.name;
		const network = NetworkLocator.findByName(Network.NETWORKS, networkName);
		const facade = new SymbolFacade(network);
		const lookupCurrency = createLookupCurrencyFunction(services.proxy);

		const parsePublicKey = rosettaPublicKey => {
			if (new CurveType().edwards25519 !== rosettaPublicKey.curve_type)
				throw RosettaErrorFactory.UNSUPPORTED_CURVE;

			try {
				return new PublicKey(rosettaPublicKey.hex_bytes);
			} catch (err) {
				throw RosettaErrorFactory.INVALID_PUBLIC_KEY;
			}
		};

		server.post('/construction/derive', rosettaPostRouteWithNetwork(networkName, ConstructionDeriveRequest, typedRequest => {
			const publicKey = parsePublicKey(typedRequest.public_key);
			const address = network.publicKeyToAddress(publicKey);

			const response = new ConstructionDeriveResponse();
			response.account_identifier = new AccountIdentifier(address.toString());
			return response;
		}));

		server.post('/construction/preprocess', rosettaPostRouteWithNetwork(networkName, ConstructionPreprocessRequest, typedRequest => {
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

		server.post('/construction/metadata', rosettaPostRouteWithNetwork(networkName, ConstructionMetadataRequest, async () => {
			// ignore request object because only global metadata is needed for transaction construction

			const results = await Promise.all([getNetworkTime(), getSuggestedTransactionMultiplier()]);

			const response = new ConstructionMetadataResponse();
			response.metadata = {
				networkTime: results[0],
				feeMultiplier: results[1]
			};
			return response;
		}));

		const buildAddressToPublicKeyMap = rosettaPublicKeys => {
			const addressToPublicKeyMap = {};
			for (let i = 0; i < rosettaPublicKeys.length; ++i) {
				const publicKey = parsePublicKey(rosettaPublicKeys[i]);
				const address = facade.network.publicKeyToAddress(publicKey);
				addressToPublicKeyMap[address.toString()] = publicKey;
			}

			return addressToPublicKeyMap;
		};

		const processTransfer = (addressToPublicKeyMap, subOperation1, subOperation2) => {
			const makeDescriptor = (senderAccount, recipientAccount, amount) => ({
				type: 'transfer_transaction_v1',
				signerPublicKey: addressToPublicKeyMap[senderAccount.address],
				recipientAddress: recipientAccount.address,
				mosaics: [
					{ mosaicId: generateMosaicAliasId('symbol.xym'), amount }
				]
			});

			const amount = BigInt(subOperation1.amount.value);
			if (0n !== amount + BigInt(subOperation2.amount.value))
				throw RosettaErrorFactory.INVALID_REQUEST_DATA;

			if (0n < amount)
				return makeDescriptor(subOperation2.account, subOperation1.account, amount);

			return makeDescriptor(subOperation1.account, subOperation2.account, -amount);
		};

		const createSigningPayload = (signingPayload, signerAddress) => {
			const result = new SigningPayload(utils.uint8ToHex(signingPayload));
			result.account_identifier = new AccountIdentifier(signerAddress.toString());
			result.signature_type = new SignatureType().ed25519;
			return result;
		};

		const extractCosignerPublicKeys = (allSignerPublicKeys, aggregateSignerPublicKey) => {
			const cosignerPublicKeyStringSet = new Set(allSignerPublicKeys.map(publicKey => publicKey.toString()));
			cosignerPublicKeyStringSet.delete(aggregateSignerPublicKey.toString());
			return [...cosignerPublicKeyStringSet].sort().map(publicKeyString => new models.PublicKey(publicKeyString));
		};

		server.post('/construction/payloads', rosettaPostRouteWithNetwork(networkName, ConstructionPayloadsRequest, async typedRequest => {
			const addressToPublicKeyMap = buildAddressToPublicKeyMap(typedRequest.public_keys);

			const embeddedTransactions = [];
			const allSignerPublicKeys = [];
			for (let i = 0; i < typedRequest.operations.length; ++i) {
				const operation = typedRequest.operations[i];

				if ('transfer' === operation.type) {
					if (i + 1 === typedRequest.operations.length)
						throw RosettaErrorFactory.INVALID_REQUEST_DATA;

					const nextOperation = typedRequest.operations[i + 1];
					if ('transfer' !== nextOperation.type)
						throw RosettaErrorFactory.INVALID_REQUEST_DATA;

					const embeddedTransaction = facade.transactionFactory.createEmbedded(processTransfer(
						addressToPublicKeyMap,
						operation,
						nextOperation
					));

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
					allSignerPublicKeys.push(embeddedTransaction.signerPublicKey);
				} else if ('cosign' === operation.type) {
					allSignerPublicKeys.push(new models.PublicKey(addressToPublicKeyMap[operation.account.address].bytes));
				}
			}

			const aggregateSignerPublicKey = new PublicKey(embeddedTransactions[0].signerPublicKey.bytes);
			const cosignerPublicKeys = extractCosignerPublicKeys(allSignerPublicKeys, aggregateSignerPublicKey);

			const merkleHash = facade.static.hashEmbeddedTransactions(embeddedTransactions);
			const aggregateTransaction = facade.transactionFactory.create({
				type: 'aggregate_complete_transaction_v2',
				signerPublicKey: aggregateSignerPublicKey,
				deadline: new NetworkTimestamp(typedRequest.metadata.networkTime).addHours(1).timestamp,
				transactionsHash: merkleHash,
				transactions: embeddedTransactions,

				cosignatures: cosignerPublicKeys.map(cosignerPublicKey => {
					const cosignature = new models.Cosignature();
					cosignature.signerPublicKey = cosignerPublicKey;
					return cosignature;
				})
			});

			const transactionSize = aggregateTransaction.size;
			aggregateTransaction.fee = new models.Amount(transactionSize * typedRequest.metadata.feeMultiplier);

			const transactionHash = facade.hashTransaction(aggregateTransaction);
			const transactionPayload = aggregateTransaction.serialize();
			const signingPayload = facade.extractSigningPayload(aggregateTransaction);

			const response = new ConstructionPayloadsResponse();
			response.unsigned_transaction = utils.uint8ToHex(transactionPayload);
			response.payloads = [
				createSigningPayload(signingPayload, network.publicKeyToAddress(aggregateTransaction.signerPublicKey)),
				...cosignerPublicKeys.map(publicKey => createSigningPayload(
					transactionHash.bytes,
					network.publicKeyToAddress(publicKey)
				))
			];

			return response;
		}));

		server.post('/construction/combine', rosettaPostRouteWithNetwork(networkName, ConstructionCombineRequest, async typedRequest => {
			const findSignature = publicKey => typedRequest.signatures.find(rosettaSignature =>
				0 === utils.deepCompare(publicKey.bytes, utils.hexToUint8(rosettaSignature.public_key.hex_bytes))).hex_bytes;

			const aggregateTransaction = facade.transactionFactory.static.deserialize(utils.hexToUint8(typedRequest.unsigned_transaction));
			aggregateTransaction.signature = new models.Signature(findSignature(aggregateTransaction.signerPublicKey));

			aggregateTransaction.cosignatures.forEach(cosignature => {
				cosignature.signature = new models.Signature(findSignature(cosignature.signerPublicKey));
			});

			const transactionPayload = aggregateTransaction.serialize();

			const response = new ConstructionCombineResponse();
			response.signed_transaction = utils.uint8ToHex(transactionPayload);
			return response;
		}));

		server.post('/construction/parse', rosettaPostRouteWithNetwork(networkName, ConstructionParseRequest, async typedRequest => {
			const currencyMosaicId = generateMosaicAliasId('symbol.xym');
			const xymCurrency = await lookupCurrency('currencyMosaicId'); // need to store resolved id in currency metadata

			const aggregateTransaction = facade.transactionFactory.static.deserialize(utils.hexToUint8(typedRequest.transaction));

			const supportedTransactionTypes = [
				models.TransactionType.MULTISIG_ACCOUNT_MODIFICATION.value,
				models.TransactionType.TRANSFER.value
			];
			if (!aggregateTransaction.transactions.every(transaction => supportedTransactionTypes.includes(transaction.type.value)))
				throw RosettaErrorFactory.NOT_SUPPORTED_ERROR;

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
					.sort((lhs, rhs) => lhs.toString().localeCompare(rhs.toString()))
					.map(address => new AccountIdentifier(address.toString()));
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

		server.post('/construction/hash', rosettaPostRouteWithNetwork(networkName, ConstructionHashRequest, async typedRequest =>
			processTransactionHashRequest(typedRequest)));

		server.post('/construction/submit', rosettaPostRouteWithNetwork(networkName, ConstructionSubmitRequest, async typedRequest => {
			await services.proxy.fetch('transactions', jsonObject => jsonObject.message, {
				method: 'PUT',
				headers: { 'Content-Type': 'application/json' },
				body: JSON.stringify({ payload: typedRequest.signed_transaction })
			});

			return processTransactionHashRequest(typedRequest);
		}));
	}
};

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
import { calculateXemTransferFee, createLookupCurrencyFunction, getBlockchainDescriptor } from './rosettaUtils.js';
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
import { PublicKey, utils } from 'symbol-sdk';
import { NemFacade, NetworkTimestamp, models } from 'symbol-sdk/nem';

export default {
	register: (server, db, services) => {
		const FEE_UNIT = 50000n;

		const blockchainDescriptor = getBlockchainDescriptor(services.config);
		const facade = new NemFacade(blockchainDescriptor.network);
		const lookupCurrency = createLookupCurrencyFunction(services.proxy);
		const publicKeyProcessor = new RosettaPublicKeyProcessor(new CurveType().edwards25519_keccak, facade.network, PublicKey);

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

					if ('multisig' === operation.type) {
						operation.metadata.addressAdditions.forEach(address => {
							response.required_public_keys.push(new AccountIdentifier(address));
						});
						operation.metadata.addressDeletions.forEach(address => {
							response.required_public_keys.push(new AccountIdentifier(address));
						});
					}
				}
			});

			return response;
		}));

		const getNetworkTime = () => services.proxy.fetch('time-sync/network-time', jsonObject => jsonObject.receiveTimeStamp);

		server.post('/construction/metadata', constructionPostRoute(ConstructionMetadataRequest, async () => {
			// ignore request object because only global metadata is needed for transaction construction

			const timestamp = await getNetworkTime();

			const response = new ConstructionMetadataResponse();
			response.metadata = {
				networkTime: Math.trunc(timestamp / 1000)
			};
			return response;
		}));

		const createSigningPayload = (signingPayload, signerAddress) => {
			const result = new SigningPayload(utils.uint8ToHex(signingPayload));
			result.account_identifier = new AccountIdentifier(signerAddress.toString());
			result.signature_type = new SignatureType().ed25519_keccak;
			return result;
		};

		server.post('/construction/payloads', constructionPostRoute(ConstructionPayloadsRequest, async typedRequest => {
			const addressToPublicKeyMap = publicKeyProcessor.buildAddressToPublicKeyMap(typedRequest.public_keys);

			const timestamp = new NetworkTimestamp(typedRequest.metadata.networkTime);
			const deadline = timestamp.addHours(1);
			const timestampProperties = {
				timestamp: timestamp.timestamp,
				deadline: deadline.timestamp
			};

			let transaction;
			const cosignerPublicKeys = [];
			for (let i = 0; i < typedRequest.operations.length; ++i) {
				const operation = typedRequest.operations[i];

				if ('transfer' === operation.type) {
					if (transaction)
						throw RosettaErrorFactory.INVALID_REQUEST_DATA;

					const transferDescriptor = extractTransferDescriptorAt(typedRequest.operations, i);
					transaction = facade.transactionFactory.create({
						type: 'transfer_transaction_v1',
						signerPublicKey: addressToPublicKeyMap[transferDescriptor.senderAddress],
						...timestampProperties,
						fee: calculateXemTransferFee(transferDescriptor.amount),

						recipientAddress: transferDescriptor.recipientAddress,
						amount: transferDescriptor.amount
					});

					++i;
				} else if ('multisig' === operation.type) {
					if (transaction)
						throw RosettaErrorFactory.INVALID_REQUEST_DATA;

					transaction = facade.transactionFactory.create({
						type: 'multisig_account_modification_transaction_v2',
						signerPublicKey: addressToPublicKeyMap[operation.account.address],
						...timestampProperties,
						fee: 10n * FEE_UNIT,

						minApprovalDelta: operation.metadata.minApprovalDelta,
						modifications: [].concat(
							operation.metadata.addressAdditions.map(address => ({
								modification: {
									modificationType: 'add_cosignatory',
									cosignatoryPublicKey: addressToPublicKeyMap[address]
								}
							})),
							operation.metadata.addressDeletions.map(address => ({
								modification: {
									modificationType: 'delete_cosignatory',
									cosignatoryPublicKey: addressToPublicKeyMap[address]
								}
							}))
						)
					});
				} else if ('cosign' === operation.type) {
					cosignerPublicKeys.push(new models.PublicKey(addressToPublicKeyMap[operation.account.address].bytes));
				}
			}

			if (0 !== cosignerPublicKeys.length) {
				const otherTransactionHash = facade.hashTransaction(transaction);
				const multisigAccountAddress = facade.network.publicKeyToAddress(transaction.signerPublicKey);

				// wrap transaction in multisig
				const signerPublicKey = cosignerPublicKeys.shift().bytes;
				transaction = facade.transactionFactory.create({
					type: 'multisig_transaction_v1',
					signerPublicKey,
					...timestampProperties,
					fee: 3n * FEE_UNIT,

					innerTransaction: facade.transactionFactory.static.toNonVerifiableTransaction(transaction),

					cosignatures: cosignerPublicKeys.map(cosignerPublicKey => {
						const cosignature = new models.SizePrefixedCosignatureV1();
						cosignature.cosignature = facade.transactionFactory.create({
							type: 'cosignature_v1',
							signerPublicKey: cosignerPublicKey.bytes,
							...timestampProperties,
							fee: 3n * FEE_UNIT,
							otherTransactionHash,
							multisigAccountAddress
						});
						return cosignature;
					})
				});
			}

			const transactionPayload = transaction.serialize();
			const signingPayload = facade.extractSigningPayload(transaction);

			const response = new ConstructionPayloadsResponse();
			response.unsigned_transaction = utils.uint8ToHex(transactionPayload);

			response.payloads = [createSigningPayload(signingPayload, facade.network.publicKeyToAddress(transaction.signerPublicKey))];
			if (0 !== cosignerPublicKeys.length) {
				response.payloads = [].concat(response.payloads, transaction.cosignatures
					.map(cosignature => cosignature.cosignature)
					.map(cosignature => {
						const cosignatureSigningPayload = facade.extractSigningPayload(cosignature);
						const cosignaturePublicKey = facade.network.publicKeyToAddress(cosignature.signerPublicKey);
						return createSigningPayload(cosignatureSigningPayload, cosignaturePublicKey);
					}));
			}

			return response;
		}));

		server.post('/construction/combine', constructionPostRoute(ConstructionCombineRequest, async typedRequest => {
			if (typedRequest.signatures.some(signature => new SignatureType().ed25519_keccak !== signature.signature_type))
				throw RosettaErrorFactory.UNSUPPORTED_CURVE;

			const findSignature = publicKey => typedRequest.signatures.find(rosettaSignature =>
				0 === utils.deepCompare(publicKey.bytes, utils.hexToUint8(rosettaSignature.public_key.hex_bytes))).hex_bytes;

			const transaction = facade.transactionFactory.static.deserialize(utils.hexToUint8(typedRequest.unsigned_transaction));

			transaction.signature = new models.Signature(findSignature(transaction.signerPublicKey));
			if (transaction.cosignatures) {
				transaction.cosignatures.map(cosignature => cosignature.cosignature).forEach(cosignature => {
					cosignature.signature = new models.Signature(findSignature(cosignature.signerPublicKey));
				});
			}

			const transactionPayload = transaction.serialize();

			const response = new ConstructionCombineResponse();
			response.signed_transaction = utils.uint8ToHex(transactionPayload);
			return response;
		}));

		server.post('/construction/parse', constructionPostRoute(ConstructionParseRequest, async typedRequest => {
			const xemCurrency = await lookupCurrency('currencyMosaicId');

			const transaction = facade.transactionFactory.static.deserialize(utils.hexToUint8(typedRequest.transaction));

			const parser = new OperationParser(facade.network, {
				lookupCurrency: mosaicId => {
					if ('currencyMosaicId' !== mosaicId)
						throw RosettaErrorFactory.NOT_SUPPORTED_ERROR;

					return xemCurrency;
				}
			});

			const transactionJson = convertTransactionSdkJsonToRestJson(transaction.toJson());
			const { operations, signerAddresses } = await parser.parseTransaction(transactionJson);

			const response = new ConstructionParseResponse();
			response.operations = operations;
			response.account_identifier_signers = [];
			response.signers = [];

			if (typedRequest.signed) {
				response.account_identifier_signers = signerAddresses
					.map(address => new AccountIdentifier(address.toString()));
			}

			return response;
		}));

		const processTransactionHashRequest = typedRequest => {
			const transaction = facade.transactionFactory.static.deserialize(utils.hexToUint8(typedRequest.signed_transaction));
			const transactionHash = facade.hashTransaction(transaction);

			const response = new TransactionIdentifierResponse();
			response.transaction_identifier = new TransactionIdentifier(transactionHash.toString());
			return response;
		};

		server.post('/construction/hash', constructionPostRoute(ConstructionHashRequest, async typedRequest =>
			processTransactionHashRequest(typedRequest)));

		const submitTransaction = async transaction => {
			const signingPayload = facade.extractSigningPayload(transaction);

			const message = await services.proxy.fetch('transaction/announce', jsonObject => jsonObject.message, {
				method: 'POST',
				headers: { 'Content-Type': 'application/json' },
				body: JSON.stringify({
					data: utils.uint8ToHex(signingPayload),
					signature: transaction.signature.toString()
				})
			});

			if ('SUCCESS' !== message)
				throw RosettaErrorFactory.INTERNAL_SERVER_ERROR;
		};

		server.post('/construction/submit', constructionPostRoute(ConstructionSubmitRequest, async typedRequest => {
			const transaction = facade.transactionFactory.static.deserialize(utils.hexToUint8(typedRequest.signed_transaction));

			if (undefined === transaction.cosignatures) {
				await submitTransaction(transaction);
			} else {
				// hoist cosignatures out of transaction
				const cosignatures = transaction.cosignatures.map(cosignature => cosignature.cosignature);
				transaction.cosignatures = [];

				await submitTransaction(transaction);
				await Promise.all(cosignatures.map(cosignature => submitTransaction(cosignature)));
			}

			return processTransactionHashRequest(typedRequest);
		}));
	}
};

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

import { calculateXemTransferFee, getBlockchainDescriptor } from './rosettaUtils.js';
import AccountIdentifier from '../openApi/model/AccountIdentifier.js';
import ConstructionDeriveRequest from '../openApi/model/ConstructionDeriveRequest.js';
import ConstructionDeriveResponse from '../openApi/model/ConstructionDeriveResponse.js';
import ConstructionMetadataRequest from '../openApi/model/ConstructionMetadataRequest.js';
import ConstructionMetadataResponse from '../openApi/model/ConstructionMetadataResponse.js';
import ConstructionPayloadsRequest from '../openApi/model/ConstructionPayloadsRequest.js';
import ConstructionPayloadsResponse from '../openApi/model/ConstructionPayloadsResponse.js';
import ConstructionPreprocessRequest from '../openApi/model/ConstructionPreprocessRequest.js';
import ConstructionPreprocessResponse from '../openApi/model/ConstructionPreprocessResponse.js';
import CurveType from '../openApi/model/CurveType.js';
import SignatureType from '../openApi/model/SignatureType.js';
import SigningPayload from '../openApi/model/SigningPayload.js';
import {
	RosettaErrorFactory, RosettaPublicKeyProcessor, extractTransferDescriptorAt, rosettaPostRouteWithNetwork
} from '../rosettaUtils.js';
import { PublicKey, utils } from 'symbol-sdk';
import { NemFacade, NetworkTimestamp, models } from 'symbol-sdk/nem';

export default {
	register: (server, db, services) => {
		const blockchainDescriptor = getBlockchainDescriptor(services.config);
		const facade = new NemFacade(blockchainDescriptor.network);
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
						fee: 50000n * 10n,

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
				// wrap transaction in multisig
				const signerPublicKey = cosignerPublicKeys.shift().bytes;
				transaction = facade.transactionFactory.create({
					type: 'multisig_transaction_v1',
					signerPublicKey,
					...timestampProperties,
					fee: transaction.fee,

					innerTransaction: transaction,

					cosignatures: cosignerPublicKeys.map(cosignerPublicKey => facade.transactionFactory.create({
						type: 'cosignature_v1',
						signerPublicKey: cosignerPublicKey.bytes,
						...timestampProperties,
						fee: 50000n * 3n
					}))
				});
			}

			const transactionHash = facade.hashTransaction(transaction);
			const transactionPayload = transaction.serialize();
			const signingPayload = facade.extractSigningPayload(transaction);

			const response = new ConstructionPayloadsResponse();
			response.unsigned_transaction = utils.uint8ToHex(transactionPayload);

			response.payloads = [
				createSigningPayload(signingPayload, facade.network.publicKeyToAddress(transaction.signerPublicKey)),
				...cosignerPublicKeys.map(publicKey => createSigningPayload(
					transactionHash.bytes,
					facade.network.publicKeyToAddress(publicKey)
				))
			];

			return response;
		}));
	}
};

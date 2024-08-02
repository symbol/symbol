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

import { getBlockchainDescriptor } from './rosettaUtils.js';
import AccountIdentifier from '../openApi/model/AccountIdentifier.js';
import ConstructionDeriveRequest from '../openApi/model/ConstructionDeriveRequest.js';
import ConstructionDeriveResponse from '../openApi/model/ConstructionDeriveResponse.js';
import ConstructionMetadataRequest from '../openApi/model/ConstructionMetadataRequest.js';
import ConstructionMetadataResponse from '../openApi/model/ConstructionMetadataResponse.js';
import ConstructionPreprocessRequest from '../openApi/model/ConstructionPreprocessRequest.js';
import ConstructionPreprocessResponse from '../openApi/model/ConstructionPreprocessResponse.js';
import CurveType from '../openApi/model/CurveType.js';
import { RosettaErrorFactory, rosettaPostRouteWithNetwork } from '../rosettaUtils.js';
import { PublicKey } from 'symbol-sdk';
import { NemFacade } from 'symbol-sdk/nem';

export default {
	register: (server, db, services) => {
		const blockchainDescriptor = getBlockchainDescriptor(services.config);
		const facade = new NemFacade(blockchainDescriptor.network);

		const parsePublicKey = rosettaPublicKey => {
			if (new CurveType().edwards25519_keccak !== rosettaPublicKey.curve_type)
				throw RosettaErrorFactory.UNSUPPORTED_CURVE;

			try {
				return new PublicKey(rosettaPublicKey.hex_bytes);
			} catch (err) {
				throw RosettaErrorFactory.INVALID_PUBLIC_KEY;
			}
		};

		const constructionPostRoute = (...args) => rosettaPostRouteWithNetwork(blockchainDescriptor, ...args);

		server.post('/construction/derive', constructionPostRoute(ConstructionDeriveRequest, typedRequest => {
			const publicKey = parsePublicKey(typedRequest.public_key);
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
	}
};

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

import AccountIdentifier from './openApi/model/AccountIdentifier.js';
import ConstructionDeriveRequest from './openApi/model/ConstructionDeriveRequest.js';
import ConstructionDeriveResponse from './openApi/model/ConstructionDeriveResponse.js';
import { errors, rosettaPostRouteWithNetwork } from './rosettaUtils.js';
import { NetworkLocator, PublicKey } from 'symbol-sdk';
import { Network } from 'symbol-sdk/symbol';

export default {
	register: (server, db, services) => {
		const networkName = services.config.network.name;
		const network = NetworkLocator.findByName(Network.NETWORKS, networkName);

		server.post('/construction/derive', rosettaPostRouteWithNetwork(networkName, ConstructionDeriveRequest, typedRequest => {
			if ('edwards25519' !== typedRequest.public_key.curve_type)
				return errors.UNSUPPORTED_CURVE;

			try {
				const publicKey = new PublicKey(typedRequest.public_key.hex_bytes);
				const address = network.publicKeyToAddress(publicKey);

				const response = new ConstructionDeriveResponse();
				response.account_identifier = new AccountIdentifier(address.toString());
				return response;
			} catch (err) {
				return errors.INVALID_PUBLIC_KEY;
			}
		}));
	}
};

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

import RosettaError from './openApi/model/Error.js';
import { sendJson } from '../../routes/simpleSend.js';

export const errors = {
	UNSUPPORTED_NETWORK: new RosettaError(1, 'unsupported network', false),
	UNSUPPORTED_CURVE: new RosettaError(2, 'unsupported curve', false),

	INVALID_PUBLIC_KEY: new RosettaError(3, 'invalid public key', false),
	INVALID_REQUEST_DATA: new RosettaError(4, 'invalid request data', false),

	CONNECTION_ERROR: new RosettaError(5, 'unable to connect to network', true)
};

export const rosettaPostRouteWithNetwork = (networkName, Request, handler) => async (req, res, next) => {
	const send = data => {
		const statusCode = data instanceof RosettaError ? 500 : undefined;
		return sendJson(res, next)(data, statusCode);
	};

	try {
		Request.validateJSON(req.body);
	} catch (err) {
		return send(errors.INVALID_REQUEST_DATA);
	}

	const typedRequest = Request.constructFromObject(req.body);
	const checkNetwork = networkIdentifier => ('Symbol' === networkIdentifier.blockchain && networkName === networkIdentifier.network);
	if (!checkNetwork(typedRequest.network_identifier))
		return send(errors.UNSUPPORTED_NETWORK);

	const response = await handler(typedRequest);
	return send(response);
};

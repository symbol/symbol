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

const parseSubscriptionRequest = request => {
	const requireString = (bag, propertyName) => {
		const str = bag[propertyName];
		if ('string' !== typeof str)
			return { error: `client subscription request (${propertyName}) must be string` };

		return str;
	};

	if (request.subscribe && request.unsubscribe)
		return { error: 'client data cannot specify both subscribe and unsubscribe' };

	if (request.unsubscribe)
		return { channel: requireString(request, 'unsubscribe'), action: 'delete' };

	return { channel: requireString(request, 'subscribe'), action: 'add' };
};

module.exports = {
	/**
	 * Handles a websocket message.
	 * @param {object} client Client that sent the message.
	 * @param {string} messageJson JSON message.
	 * @param {object} subscriptionManager Subscription manager.
	 * @returns {array} Error information or undefined if no error occurred.
	 */
	handleMessage: (client, messageJson, subscriptionManager) => {
		let request;
		try {
			request = JSON.parse(messageJson);
		} catch (err) {
			return [`parse error for data ${messageJson}`, err];
		}

		// check if uid matches assigned one
		if (request.uid !== client.uid)
			return [`client data does not have proper uid: ${messageJson}`];

		const subscriptionRequest = parseSubscriptionRequest(request);
		const parseErrorMessage = subscriptionRequest.error || subscriptionRequest.channel.error;
		if (parseErrorMessage)
			return [parseErrorMessage];

		try {
			subscriptionManager[subscriptionRequest.action](subscriptionRequest.channel, client);
		} catch (err) {
			return [`subscribe error for data ${messageJson}`, err];
		}

		return undefined;
	}
};

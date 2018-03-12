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
	 * @param {object} client The client that sent the message.
	 * @param {string} messageJson The JSON message.
	 * @param {object} subscriptionManager The subscription manager.
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

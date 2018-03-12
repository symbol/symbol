const zmq = require('zmq');
const zmqUtils = require('./zmqUtils');

const createZmqSocket = (key, zmqConfig, logger) => {
	const zsocket = zmq.socket('sub');
	zsocket.key = key;
	zmqUtils.prepareZsocket(zsocket, zmqConfig, logger);

	zsocket.connect(`tcp://${zmqConfig.host}:${zmqConfig.port}`);
	return zsocket;
};

const findSubscriptionInfo = (key, emitter, codec, channelDescriptors) => {
	const [topicCategory, topicParam] = key.split('/');
	if (!(topicCategory in channelDescriptors))
		throw new Error(`unknown topic category ${topicCategory}`);

	const descriptor = channelDescriptors[topicCategory];
	const handler = descriptor.handler(codec, data => { emitter.emit(key, data); });
	const filter = descriptor.filter(topicParam);
	return { filter, handler };
};

/**
 * Service for creating channel-specific zmq sockets.
 * @param {object} zmqConfig Configuration for configuring sockets.
 * @param {object} codec Codec used to deserialize zmq messages.
 * @param {object} channelDescriptors Registered message channel descriptors.
 * @param {object} logger Level-based logger object.
 * @returns {object} Newly created zmq connection service that is a stripped down EventEmitter.
 */
module.exports.createZmqConnectionService = (zmqConfig, codec, channelDescriptors, logger) =>
	zmqUtils.createMultisocketEmitter((key, emitter) => {
		logger.info(`subscribing to ${key}`);
		const subscriptionInfo = findSubscriptionInfo(key, emitter, codec, channelDescriptors);

		const zsocket = createZmqSocket(key, zmqConfig, logger);
		zsocket.subscribe(subscriptionInfo.filter);
		zsocket.on('message', subscriptionInfo.handler);
		return zsocket;
	});

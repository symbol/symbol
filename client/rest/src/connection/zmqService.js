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

import zmqUtils from './zmqUtils.js';
import zmq from 'zeromq';
import EventEmitter from 'events';

// Map v6 event types to v5-compatible event names used by logAllMonitorEvents
const V6_TO_V5_EVENT_NAME_MAP = new Map([
	['connect', 'connect'],
	['connect:delay', 'connect_delay'],
	['connect:retry', 'connect_retry'],
	['bind', 'listen'],
	['bind:error', 'bind_error'],
	['accept', 'accept'],
	['accept:error', 'accept_error'],
	['close', 'close'],
	['close:error', 'close_error'],
	['disconnect', 'disconnect']

]);

class ZmqSocketWrapper extends EventEmitter {
	constructor(key, subscriberFactory) {
		super();
		this.key = key;
		this.innerSocket = subscriberFactory ? subscriberFactory() : new zmq.Subscriber();
		this.innerSocket.linger = 0;
		this.eventsLoopActive = false;
	}

	connect(url) {
		this.innerSocket.connect(url);
	}

	subscribe(filter) {
		this.innerSocket.subscribe(filter);
	}

	monitor() {
		this.eventsLoopActive = true;
		(async () => {
			try {
				while (this.eventsLoopActive) {
					const event = await this.innerSocket.events.receive(); // eslint-disable-line no-await-in-loop
					const v5Name = V6_TO_V5_EVENT_NAME_MAP.get(event.type);
					if (v5Name)
						this.emit(v5Name, event.value, event.address);
				}
			} catch (err) {
				console.error('Error in ZMQ event loop:', err);
				if (this.eventsLoopActive)
					this.emit('error', err);
			}
		})();
	}

	unmonitor() {
		this.eventsLoopActive = false;
	}

	startReceiving(handler) {
		(async () => {
			try {
				while (!this.innerSocket.closed) {
					const frames = await this.innerSocket.receive(); // eslint-disable-line no-await-in-loop
					handler(...frames);
				}
			} catch (err) {
				if (this.eventsLoopActive)
					this.emit('error', err);
			}
		})();
	}

	close() {
		this.eventsLoopActive = false;
		try {
			this.innerSocket.close();
		} catch (err) {
			// ignore errors during close
		}
	}
}

export { ZmqSocketWrapper };

const createZmqSocket = (key, zmqConfig, logger, currentSocketCount) => {
	const zsocket = new ZmqSocketWrapper(key, () => new zmq.Subscriber());
	zmqUtils.prepareZsocket(zsocket, zmqConfig, logger);

	zsocket.connect(`tcp://${zmqConfig.host}:${zmqConfig.port}`);
	logger.info(`Current zmq subscription count: ${currentSocketCount + 1}`);
	return zsocket;
};

const findSubscriptionInfo = (key, emitter, channelDescriptors) => {
	const [topicCategory, topicParam] = key.split('/');
	if (!(topicCategory in channelDescriptors))
		throw new Error(`unknown topic category ${topicCategory}`);

	const descriptor = channelDescriptors[topicCategory];
	const handler = descriptor.handler(data => { emitter.emit(key, data); });
	const filter = descriptor.filter(topicParam);
	return { filter, handler };
};

/**
 * Service for creating channel-specific zmq sockets.
 * @param {object} zmqConfig Configuration for configuring sockets.
 * @param {object} channelDescriptors Registered message channel descriptors.
 * @param {object} logger Level-based logger object.
 * @returns {object} Newly created zmq connection service that is a stripped down EventEmitter.
 */
export default (zmqConfig, channelDescriptors, logger) =>
	zmqUtils.createMultisocketEmitter((key, emitter, currentSocketCount) => {
		if (currentSocketCount === (!zmqConfig.maxSubscriptions ? 500 : zmqConfig.maxSubscriptions))
			throw new Error('Max subscriptions reached.');

		logger.info(`subscribing to ${key}`);
		const subscriptionInfo = findSubscriptionInfo(key, emitter, channelDescriptors);

		const zsocket = createZmqSocket(key, zmqConfig, logger, currentSocketCount);
		// the second param (handler) gets called with the provided args in the message, which vary depending on the defined handler type
		// (block, transaction, transactionStatus...)
		zsocket.subscribe(subscriptionInfo.filter);
		zsocket.startReceiving(subscriptionInfo.handler);
		return zsocket;
	});

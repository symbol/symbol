/*
 * Copyright (c) 2016-present,
 * Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

const errors = require('./errors');
const restify = require('restify');
const restifyErrors = require('restify-errors');
const SubscriptionManager = require('./SubscriptionManager');
const WebSocket = require('ws');
const websocketMessageHandler = require('./websocketMessageHandler');
const websocketUtils = require('./websocketUtils');
const winston = require('winston');

const isPromise = object => object && object.catch;

const toRestError = err => {
	const restError = errors.toRestError(err);
	winston.error(`caught error ${restError.statusCode}`, restError);
	return restError;
};

const createCrossDomainHeaderAdder = crossDomainHttpMethods => {
	const allowMethods = crossDomainHttpMethods.join(',');
	return (requestMethod, res) => {
		if (crossDomainHttpMethods.some(method => method === requestMethod)) {
			res.header('Access-Control-Allow-Origin', '*');
			res.header('Access-Control-Allow-Methods', allowMethods);
			res.header('Access-Control-Allow-Headers', 'Content-Type');
		}
	};
};

const catapultRestifyPlugins = {
	crossDomain: addCrossDomainHeaders => (req, res, next) => {
		addCrossDomainHeaders(req.method, res);
		next();
	},
	body: () => (req, res, next) => {
		// reject any GET or OPTIONS request with a body
		const mediaType = req.contentType().toLowerCase();
		if (['GET', 'OPTIONS'].includes(req.method)) {
			if (!req.contentLength())
				next();
			else
				next(new restifyErrors.UnsupportedMediaTypeError(mediaType));

			return;
		}

		// for other HTTP methods reject mismatched media types even if body is empty
		if ('application/json' !== mediaType) {
			next(new restifyErrors.UnsupportedMediaTypeError(mediaType));
			return;
		}

		next();
	}
};

module.exports = {
	/**
	 * Creates a REST api server.
	 * @param {array} crossDomainHttpMethods The HTTP methods that are allowed to be accessed cross-domain.
	 * @param {object} formatters The formatters to use for formatting responses.
	 * @returns {object} The server.
	 */
	createServer: (crossDomainHttpMethods, formatters) => {
		// create the server using a custom formatter
		const server = restify.createServer({
			name: '', // disable server header in response
			formatters: {
				'application/json': formatters.json
			}
		});

		// only allow application/json
		const addCrossDomainHeaders = createCrossDomainHeaderAdder(crossDomainHttpMethods || []);
		server.pre(catapultRestifyPlugins.body());

		server.use(catapultRestifyPlugins.crossDomain(addCrossDomainHeaders));
		server.use(restify.plugins.acceptParser('application/json'));
		server.use(restify.plugins.queryParser({ mapParams: true }));
		server.use(restify.plugins.jsonBodyParser({ mapParams: true }));

		// make the server promise aware (only a subset of HTTP methods are supported)
		const routeDescriptors = [];
		const promiseAwareServer = {
			listen: port => {
				// sort routes by route name in descending order (catapult is only using string routes) in order to ensure that
				// exact match routes (e.g. /foo/fixed) take precedence over wildcard routes (e.g. /foo/:variable)
				routeDescriptors.sort((lhs, rhs) => {
					if (lhs.route === rhs.route)
						return 0;

					return lhs.route < rhs.route ? 1 : -1;
				});
				routeDescriptors.forEach(descriptor => {
					server[descriptor.method](descriptor.route, descriptor.handler);
				});

				return server.listen(port);
			}
		};

		['get', 'put', 'post'].forEach(method => {
			promiseAwareServer[method] = (route, handler) => {
				const promiseAwareHandler = (req, res, next) => {
					try {
						const result = handler(req, res, next);
						if (!isPromise(result))
							return;

						result.catch(err => {
							next(toRestError(err));
						});
					} catch (err) {
						next(toRestError(err));
					}
				};

				routeDescriptors.push({ method, route, handler: promiseAwareHandler });
			};
		});

		server.on('MethodNotAllowed', (req, res) => {
			if ('OPTIONS' === req.method) {
				// notice that headers need to be added explicitly because catapultRestifyPlugins.crossDomain is not called after errors
				addCrossDomainHeaders(req.method, res);
				return res.send(204);
			}

			// fallback to default behavior
			return res.send(new restifyErrors.MethodNotAllowedError(`${req.method} is not allowed`));
		});

		// handle upgrade events (for websocket support)
		const wss = new WebSocket.Server({ noServer: true, clientTracking: false });

		server.on('upgrade', (req, socket, head) => {
			wss.handleUpgrade(req, socket, head, client => {
				wss.emit(`connection${req.url}`, client);
			});
		});

		const clientGroups = [];
		promiseAwareServer.ws = (route, callbacks) => {
			const subscriptionManager = new SubscriptionManager(Object.assign({}, callbacks, {
				newChannel: (channel, subscribers) =>
					callbacks.newChannel(channel, websocketUtils.createMultisender(subscribers, formatters.ws))
			}));

			const clients = new Set();
			clientGroups.push({ clients, subscriptionManager });

			wss.on(`connection${route}`, client => {
				const messageHandler = messageJson => websocketMessageHandler.handleMessage(client, messageJson, subscriptionManager);
				websocketUtils.handshake(client, messageHandler);

				winston.verbose(`websocket ${client.uid}: created ${route} websocket connection`);
				clients.add(client);

				client.on('close', () => {
					subscriptionManager.deleteClient(client);
					clients.delete(client);
					winston.verbose(`websocket ${client.uid}: disconnected ${route} websocket connection`);
				});
			});
		};

		promiseAwareServer.close = () => {
			// close all connected websockets
			clientGroups.forEach(clientGroup => clientGroup.clients.forEach(client => {
				client.terminate();
			}));

			// close the servers
			wss.close();
			server.close();
		};

		return promiseAwareServer;
	}
};

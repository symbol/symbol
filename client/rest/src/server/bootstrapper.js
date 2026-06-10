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

import SubscriptionManager from './SubscriptionManager.js';
import errors from './errors.js';
import websocketMessageHandler from './websocketMessageHandler.js';
import websocketUtils from './websocketUtils.js';
import accepts from '@fastify/accepts';
import rateLimit from '@fastify/rate-limit';
import Fastify from 'fastify';
import winston from 'winston';
import { WebSocketServer } from 'ws';
import fs from 'fs';

const toRestError = err => {
	const restError = errors.toRestError(err);
	winston.error(`caught error ${restError.statusCode}`, restError);
	return restError;
};

const createCrossDomainHeaderAdder = crossDomainConfig => (req, res) => {
	if (!req.headers.origin || !crossDomainConfig)
		return;

	const crossDomainResponseHeaders = {};
	if (crossDomainConfig.allowedMethods.includes(req.method))
		crossDomainResponseHeaders['Access-Control-Allow-Methods'] = crossDomainConfig.allowedMethods.join(',');

	if (!crossDomainConfig.allowedHosts.includes('*')) {
		if (crossDomainConfig.allowedHosts.includes(req.headers.origin))
			crossDomainResponseHeaders['Access-Control-Allow-Origin'] = req.headers.origin;
	} else {
		crossDomainResponseHeaders['Access-Control-Allow-Origin'] = '*';
	}

	crossDomainResponseHeaders['Access-Control-Allow-Headers'] = 'Content-Type';

	if (3 <= Object.keys(crossDomainResponseHeaders).length) {
		if ('*' !== crossDomainResponseHeaders['Access-Control-Allow-Origin'])
			crossDomainResponseHeaders.Vary = 'Origin';

		Object.keys(crossDomainResponseHeaders).forEach(header => res.header(header, crossDomainResponseHeaders[header]));
	}
};

const TEXT_PLAIN_PATHS = [
	'/network/currency/supply/circulating',
	'/network/currency/supply/max',
	'/network/currency/supply/total'
];

const catapultPlugins = {
	crossDomain: addCrossDomainHeaders => async (request, reply) => {
		// OPTIONS CORS headers are added by the dedicated OPTIONS wildcard handler, not here,
		// so they are only included in successful (2xx) responses and not in 404 responses
		if ('OPTIONS' === request.method)
			return;

		addCrossDomainHeaders(request, reply);
	},
	body: () => async request => {
		const mediaType = (request.headers['content-type'] || '').split(';')[0].trim().toLowerCase();
		if (['GET', 'OPTIONS'].includes(request.method)) {
			const len = parseInt(request.headers['content-length'] || '0', 10);
			if (0 < len)
				throw errors.createUnsupportedMediaTypeError(mediaType);

			return;
		}

		if ('application/json' !== mediaType)
			throw errors.createUnsupportedMediaTypeError(mediaType);
	},
	acceptParser: () => async request => {
		const urlPath = request.url.split('?')[0];
		const isTextPlainPath = TEXT_PLAIN_PATHS.includes(urlPath);
		const acceptType = isTextPlainPath ? 'text/plain' : 'application/json';
		if (!request.accepts().type(acceptType)) {
			const requiredType = isTextPlainPath ? 'text/plain' : 'application/json';
			throw errors.createNotAcceptableError(`Endpoint accepts only ${requiredType}`);
		}
	}
};

const readSSLFileSync = (path, fileType, pathProperty) => {
	if (!path)
		throw new Error(`No SSL ${fileType} found, '${pathProperty}' property in the configuration must be provided.`);

	try {
		return fs.readFileSync(path);
	} catch (err) {
		if ('ENOENT' === err.code)
			throw new Error(`SSL ${fileType} file cannot be found at the path: ${path}`);

		throw err;
	}
};

const HTTP_METHODS = ['GET', 'POST', 'PUT'];
export default {
	createCrossDomainHeaderAdder,

	/**
	 * Creates a REST api server.
	 * @param {object} config Application configuration (see rest.json).
	 * @param {object} formatters Formatters to use for formatting responses.
	 * @param {object} throttlingConfig Throttling configuration parameters, if not provided throttling won't be enabled.
	 * @returns {object} Server.
	 */
	createServer: (config, formatters, throttlingConfig) => {
		if (!config)
			throw new Error('Config must be provided!');

		if (!config.protocol)
			winston.warn('Protocol(HTTPS|HTTP) is not configured explicitly in the configuration, defaulting to HTTPS.');

		const protocol = config.protocol || 'HTTPS';
		winston.info(`Using protocol: ${protocol}`);

		let httpsOptions = null;
		if ('HTTPS' === protocol) {
			httpsOptions = {
				key: readSSLFileSync(config.sslKeyPath, 'Key', 'sslKeyPath'),
				cert: readSSLFileSync(config.sslCertificatePath, 'Certificate', 'sslCertificatePath')
			};
		}

		const server = Fastify({
			https: httpsOptions,
			disableRequestLogging: true,
			trustProxy: config.trustProxy || false
		});
		server.register(accepts);

		// rate limiting
		if (throttlingConfig) {
			if (throttlingConfig.max && throttlingConfig.timeWindow) {
				winston.warn(`Registering throttling ${throttlingConfig.timeWindow}ms max: ${throttlingConfig.max}`);
				server.register(rateLimit, {
					global: true,
					max: throttlingConfig.max,
					timeWindow: throttlingConfig.timeWindow,
					allowList: throttlingConfig.allowList || [],
					keyGenerator: request => request.ip
				});
			} else {
				winston.warn('throttling was not enabled - configuration is invalid or incomplete');
			}
		}

		// config.crossDomain: Configuration related to access control, contains allowed host and HTTP methods.
		if (!config.crossDomain)
			winston.warn('CORS was not enabled - configuration incomplete');

		// make the server promise aware (only a subset of HTTP methods are supported)
		const routeDescriptors = [];
		const fastifyRegisterHooks = fastify => {
			// Override the default JSON body parser to accept empty bodies gracefully
			// (Fastify v5 otherwise returns 400 for PUT/POST with Content-Type: application/json but no body)
			fastify.addContentTypeParser('application/json', { parseAs: 'string' }, (req, body, done) => {
				try {
					done(null, body ? JSON.parse(body) : {});
				} catch (err) {
					const parseError = new Error(`Invalid JSON body: ${err.message}`);
					parseError.statusCode = 400;
					done(parseError, undefined);
				}
			});

			fastify.addHook('onRequest', catapultPlugins.body());
			const addCrossDomainHeaders = createCrossDomainHeaderAdder(config.crossDomain || {});
			fastify.addHook('onRequest', catapultPlugins.crossDomain(addCrossDomainHeaders));
			fastify.addHook('onRequest', catapultPlugins.acceptParser());

			// custom error handler — covers hook errors and route errors
			fastify.setErrorHandler((error, request, reply) => {
				const restError = toRestError(error);
				reply.code(restError.statusCode)
					.type('application/json')
					.send(restError.body || { message: restError.message });
			});

			// Merge path params + query + body into request.params (mirrors restify's mapParams:true)
			fastify.addHook('preHandler', async request => {
				request.params = Object.assign(
					{},
					request.params,
					request.query,
					request.body && 'object' === typeof request.body ? request.body : {}
				);
			});

			// Format catapult payload envelopes before serialization
			fastify.addHook('preSerialization', async (request, reply, payload) => {
				if ('object' !== typeof payload || null === payload || Buffer.isBuffer(payload))
					return payload;

				const formatterName = payload.formatter;
				if (undefined !== formatterName)
					delete payload.formatter;

				const formatter = formatters[formatterName || 'json'];
				if (!formatter)
					return payload;

				reply.type('application/json');
				const jsonString = 'ws' === formatterName ? formatter(payload) : formatter(request, reply, payload);

				// jsonString is already a serialized JSON string; bypass Fastify's own
				reply.serializer(s => s);
				return jsonString;
			});

			const getAllowedMethodsForUrl = url => HTTP_METHODS.filter(method => null !== fastify.findRoute({ method, url }));

			// Not-found handler: returns 405 for method-not-allowed, 404 for unknown paths.
			const rateLimitHandler = throttlingConfig && throttlingConfig.max && throttlingConfig.timeWindow
				? { preHandler: fastify.rateLimit() }
				: {};
			fastify.setNotFoundHandler(rateLimitHandler, (request, reply) => {
				const urlPath = request.url.split('?')[0];
				const methods = getAllowedMethodsForUrl(urlPath);
				if (0 < methods.length) {
					if ('OPTIONS' === request.method) {
						addCrossDomainHeaders(request, reply);
						reply.header('allow', methods.join(', '));
						reply.code(204).send('');
					} else {
						reply.header('allow', methods.join(', '));
						reply.code(405).type('application/json').send({
							code: 'MethodNotAllowed',
							message: `${request.method} is not allowed`
						});
					}
					return;
				}

				reply.code(404).type('application/json').send({
					code: 'ResourceNotFound', message: `${urlPath} does not exist`
				});
			});
		};

		const promiseAwareServer = {
			/**
			 * Starts the HTTP server. Returns a promise resolving to the underlying Node.js server.
			 * @param {number} port TCP port to listen on (0 = OS-assigned).
			 * @returns {Promise<object>} Resolves with the underlying `http.Server` instance.
			 */
			listen: async port => {
				// sort routes by route name in descending order (catapult is only using string routes) in order to ensure that
				// exact match routes (e.g. /foo/fixed) take precedence over wildcard routes (e.g. /foo/:variable)
				routeDescriptors.sort((lhs, rhs) => {
					if (lhs.route === rhs.route)
						return 0;

					return lhs.route < rhs.route ? 1 : -1;
				});

				// wait for all plugins to be ready before registering routes, to ensure that hooks are properly applied
				await server.after();
				fastifyRegisterHooks(server);
				routeDescriptors.forEach(descriptor => {
					server[descriptor.method](descriptor.route, descriptor.handler);
				});

				await server.listen({ port, host: '0.0.0.0' });
				return server.server;
			}
		};

		HTTP_METHODS.map(method => method.toLowerCase()).forEach(method => {
			promiseAwareServer[method] = (route, handler) => {
				routeDescriptors.push({ method, route, handler });
			};
		});

		// handle upgrade events (for websocket support)
		const wss = new WebSocketServer({ noServer: true, clientTracking: false });

		// attach the WS upgrade handler to the underlying Node.js server once Fastify is ready
		server.ready(() => {
			server.server.on('upgrade', (req, socket, head) => {
				wss.handleUpgrade(req, socket, head, client => {
					wss.emit(`connection${req.url}`, client);
				});
			});
		});

		const clientGroups = [];
		promiseAwareServer.ws = (route, callbacks) => {
			const subscriptionManager = new SubscriptionManager(Object.assign({}, callbacks, {
				newChannel: (channel, subscribers) =>
					callbacks.newChannel(channel, websocketUtils.createMultisender(channel, subscribers, formatters.ws))
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
			server.close(err => winston.info(`Server closed${err ? `: ${err}` : ''}`));
		};

		return promiseAwareServer;
	}
};

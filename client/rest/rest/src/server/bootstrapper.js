import restify from 'restify';
import winston from 'winston';
import WebSocket from 'ws';
import errors from './errors';

function isPromise(object) {
	return object && object.catch;
}

function toRestError(err) {
	const restError = errors.toRestError(err);
	winston.error(`caught error ${restError.statusCode}: ${restError.message}`);
	return restError;
}

function createCrossDomainHandler(crossDomainHttpMethods) {
	const allowMethods = crossDomainHttpMethods.join(',');
	return (req, res, next) => {
		if (crossDomainHttpMethods.some(method => method === req.route.method)) {
			res.header('Access-Control-Allow-Origin', '*');
			res.header('Access-Control-Allow-Methods', allowMethods);
			res.header('Access-Control-Allow-Headers', 'Content-Type');
		}

		next();
	};
}

export default {
	/**
	 * Creates a REST api server.
	 * @param {array} crossDomainHttpMethods The HTTP methods that are allowed to be accessed cross-domain.
	 * @param {object} formatters The formatters to use for formatting responses.
	 * @returns {object} The server.
	 */
	createServer: (crossDomainHttpMethods, formatters) => {
		// create the server using a custom formatter
		const server = restify.createServer({
			formatters: {
				'application/json': formatters.json
			}
		});

		// only allow application/json
		server.use(createCrossDomainHandler(crossDomainHttpMethods || []));
		server.use(restify.acceptParser('application/json'));
		server.use(restify.queryParser());
		server.use(restify.bodyParser({ rejectUnknown: true }));

		// make the server promise aware (only GET and PUT are supported)
		const promiseAwareServer = {
			listen: port => server.listen(port)
		};

		for (const method of ['get', 'put', 'post']) {
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

				server[method](route, promiseAwareHandler);
			};
		}

		// handle upgrade events (for websocket support)
		const wss = new WebSocket.Server({ noServer: true, clientTracking: false });

		server.on('upgrade', (req, socket, head) => {
			wss.handleUpgrade(req, socket, head, client => {
				wss.emit(`connection${req.url}`, client);
			});
		});

		const clientGroups = [];
		promiseAwareServer.ws = (route, handler) => {
			let id = 0;
			const clients = new Set();
			clientGroups.push(clients);
			wss.on(`connection${route}`, client => {
				const clientId = ++id;
				clients.add(client);
				winston.verbose(`created ${route} websocket connection ${clientId}`);

				client.on('close', () => {
					clients.delete(client);
					winston.verbose(`disconnected ${route} websocket connection ${clientId}`);
				});
			});

			handler({
				send: data => {
					const view = formatters.ws(data);
					for (const client of clients) {
						client.send(view, err => {
							if (err) {
								winston.error(`error sending data to websocket: ${err.message}`);
								client.close();
							}
						});
					}
				}
			});
		};

		promiseAwareServer.close = () => {
			// close all connected websockets
			for (const clients of clientGroups) {
				for (const client of clients)
					client.terminate();
			}

			// close the servers
			wss.close();
			server.close();
		};

		return promiseAwareServer;
	}
};

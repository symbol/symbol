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

import sinon from 'sinon';

export default class MockServer {
	constructor() {
		this.routes = {};
		this.server = {};
		['get', 'put', 'post'].forEach(method => {
			this.server[method] = (path, handler) => {
				this.routes[path] = this.routes[path] || {};
				this.routes[path][method] = () => handler;
			};
		});

		this.done = sinon.fake();
		this.send = sinon.fake();
		this.redirect = sinon.fake();
		this.status = sinon.fake();
		this.header = sinon.fake();

		// Native Fastify-style reply mock
		const self = this;
		this.reply = {
			// Calling send finalizes the response: track the payload and advance the pipeline
			send: body => {
				self.send(body);
				self.done();
				return self.reply;
			},
			code: code => {
				self.status(code);
				self.reply.statusCode = code;
				return self.reply;
			},
			type: () => self.reply,
			header: (key, value) => {
				self.header(key, value);
				return self.reply;
			},
			redirect: url => {
				self.redirect(url);
				self.done();
				return self.reply;
			}
		};

		// Keep res alias pointing at reply for backward compatibility in tests
		this.res = this.reply;
	}

	resetStats() {
		this.done.resetHistory();
		this.send.resetHistory();
		this.redirect.resetHistory();
	}

	getRoute(path) {
		return this.routes[path];
	}

	callRoute(route, req) {
		try {
			const result = route(req, this.reply);
			// For async routes: only forward unhandled rejections to done(err)
			// (handled errors are routed through reply.send which already calls done)
			return Promise.resolve(result).catch(err => {
				this.done(err);
			});
		} catch (err) {
			// Synchronous throw: forward to done(err) and return resolved Promise
			this.done(err);
			return Promise.resolve();
		}
	}
}

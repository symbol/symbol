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

		this.next = sinon.fake();
		this.send = sinon.fake();
		this.redirect = sinon.fake();
		this.status = sinon.fake();
		this.setHeader = sinon.fake();
		this.res = {
			send: this.send,
			redirect: this.redirect,
			status: this.status,
			setHeader: this.setHeader
		};
	}

	resetStats() {
		this.next.resetHistory();
		this.send.resetHistory();
		this.redirect.resetHistory();
	}

	getRoute(path) {
		return this.routes[path];
	}

	callRoute(route, req) {
		return route(req, this.res, this.next);
	}
}

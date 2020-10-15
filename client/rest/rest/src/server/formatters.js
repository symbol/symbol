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

const catapult = require('catapult-sdk');

const { formatArray, formatPage } = catapult.utils.formattingUtils;

const isCatapultObject = body => body && body.payload && body.type;

const formatBody = (modelFormatter, body) => {
	const formatCatapultObject = (payload, type, structure, wsTopic) => {
		if (Array.isArray(payload))
			return formatArray(modelFormatter[type], payload);

		if ('page' === structure)
			return formatPage(modelFormatter[type], payload);

		if (wsTopic) {
			return {
				topic: wsTopic,
				data: modelFormatter[type].format(payload)
			};
		}

		return modelFormatter[type].format(payload);
	};

	let view = body;
	let statusCode;
	if (body instanceof Error) {
		// snoop for RestError or HttpError, but don't rely on instanceof
		statusCode = body.statusCode || 500;
		view = body.body ? body.body : { message: body.message };
	} else if (isCatapultObject(body)) {
		view = formatCatapultObject(body.payload, body.type, body.structure, body.topic);
	}

	return {
		statusCode,
		json: JSON.stringify(view)
	};
};

module.exports = {
	/**
	 * Creates server formatters around a model formatter.
	 * @param {array<object>} modelFormatters Model formatters.
	 * @returns {object} Server formatters.
	 */
	create: modelFormatters => ({
		/**
		 * Restify compatible formatter for JSON responses.
		 * @param {object} req Request.
		 * @param {object} res Response.
		 * @param {object} body Body.
		 * @returns {object} Result of the callback.
		 */
		json: (req, res, body) => {
			// implementation based on https://github.com/restify/node-restify/blob/4.x/lib/formatters/json.js
			const formatter = (body && body.formatter !== undefined) ? modelFormatters[body.formatter] : modelFormatters.json;
			if (body)
				delete body.formatter;
			const view = formatBody(formatter, body);
			if (view.statusCode)
				res.statusCode = view.statusCode;

			res.setHeader('Content-Length', Buffer.byteLength(view.json));
			return view.json;
		},

		/**
		 * Websocket formatter.
		 * @param {object} body Body.
		 * @returns {object} Formatted body.
		 */
		ws: body => (isCatapultObject(body) && 'raw' === body.type ? body.payload : formatBody(modelFormatters.ws, body).json)
	})
};

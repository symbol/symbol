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

const sendUnformatted = (res, next, contentType, description) => (data, statusCode) => {
	if (!data)
		throw Error(`error retrieving ${description}`);

	if (statusCode)
		res.statusCode = statusCode;

	res.setHeader('content-type', contentType);
	res.send(data);

	next();
};

/**
 * Creates a data handler that forwards a plain text result.
 * @param {object} res Restify response object.
 * @param {Function} next Restify next callback handler.
 * @returns {Function} An appropriate object handler.
 */
export const sendPlainText = (res, next) => sendUnformatted(res, next, 'text/plain', 'plain text');

/**
 * Creates a data handler that forwards a JSON object that bypasses the formatting subsystem.
 * @param {object} res Restify response object.
 * @param {Function} next Restify next callback handler.
 * @returns {Function} An appropriate object handler.
 */
export const sendJson = (res, next) => sendUnformatted(res, next, 'application/json', 'JSON object');

/**
 * Creates a data handler that forwards binary data result.
 * @param {object} res Restify response object.
 * @param {Function} next Restify next callback handler.
 * @returns {Function} An appropriate object handler.
 */
export const sendMetalData = (res, next) => {
	const isAttachment = (download, mimeType) => 'true' === download || 'application/octet-stream' === mimeType;
	return (data, mimeType, fileName, text, download) => {
		res.setHeader('content-type', mimeType);
		let disposition = isAttachment(download, mimeType) ? 'attachment;' : 'inline;';
		disposition += fileName ? ` filename="${fileName}"` : '';
		res.setHeader('Content-Disposition', disposition);
		if (text)
			res.setHeader('Content-MetalText', text);
		res.write(data);
		res.end();
		next();
	};
};

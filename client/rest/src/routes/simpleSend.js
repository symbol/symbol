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

const sendUnformatted = (reply, contentType, description) => (data, statusCode) => {
	if (!data)
		throw Error(`error retrieving ${description}`);

	if (statusCode)
		reply.code(statusCode);

	return reply.type(contentType).send(data);
};

/**
 * Creates a data handler that forwards a plain text result.
 * @param {object} reply Fastify reply object.
 * @returns {Function} An appropriate object handler.
 */
export const sendPlainText = reply => sendUnformatted(reply, 'text/plain', 'plain text');

/**
 * Creates a data handler that forwards a JSON object that bypasses the formatting subsystem.
 * @param {object} reply Fastify reply object.
 * @returns {Function} An appropriate object handler.
 */
export const sendJson = reply => sendUnformatted(reply, 'application/json', 'JSON object');

/**
 * Creates a data handler that forwards binary data result.
 * @param {object} reply Fastify reply object.
 * @returns {Function} An appropriate object handler.
 */
export const sendMetalData = reply => {
	const isAttachment = (download, mimeType) => 'true' === download || 'application/octet-stream' === mimeType;
	return (data, mimeType, fileName, text, download) => {
		reply.header('content-type', mimeType);
		let disposition = isAttachment(download, mimeType) ? 'attachment;' : 'inline;';
		disposition += fileName ? ` filename="${fileName}"` : '';
		reply.header('Content-Disposition', disposition);
		if (text)
			reply.header('Content-MetalText', text);

		return reply.send(data);
	};
};

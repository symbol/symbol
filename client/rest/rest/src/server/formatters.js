const catapult = require('catapult-sdk');

const { formatArray } = catapult.utils.formattingUtils;

const isCatapultObject = body => body && body.payload && body.type;

const formatBody = (modelFormatter, body) => {
	const formatCatapultObject = (payload, type) =>
		(!Array.isArray(payload) ? modelFormatter[type].format(payload) : formatArray(modelFormatter[type], payload));

	let view = body;
	let statusCode;
	if (body instanceof Error) {
		// snoop for RestError or HttpError, but don't rely on instanceof
		statusCode = body.statusCode || 500;
		view = body.body ? body.body : { message: body.message };
	} else if (isCatapultObject(body)) {
		view = formatCatapultObject(body.payload, body.type);
	}

	return {
		statusCode,
		json: JSON.stringify(view)
	};
};

module.exports = {
	/**
	 * Creates server formatters around a model formatter.
	 * @param {array<object>} modelFormatters The model formatters.
	 * @returns {object} The server formatters.
	 */
	create: modelFormatters => ({
		/**
		 * Restify compatible formatter for JSON responses.
		 * @param {object} req The request.
		 * @param {object} res The response.
		 * @param {object} body The body.
		 * @returns {object} The result of the callback.
		 */
		json: (req, res, body) => {
			// implementation based on https://github.com/restify/node-restify/blob/4.x/lib/formatters/json.js
			const view = formatBody(modelFormatters.json, body);
			if (view.statusCode)
				res.statusCode = view.statusCode;

			res.setHeader('Content-Length', Buffer.byteLength(view.json));
			return view.json;
		},

		/**
		 * Websocket formatter.
		 * @param {object} body The body.
		 * @returns {object} The formatted body.
		 */
		ws: body => (isCatapultObject(body) && 'raw' === body.type ? body.payload : formatBody(modelFormatters.ws, body).json)
	})
};

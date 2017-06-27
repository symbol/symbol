import catapult from 'catapult-sdk';

const formatArray = catapult.utils.formattingUtils.formatArray;

function isCatapultObject(body) {
	return body && body.payload && body.type;
}

function formatBody(modelFormatter, body) {
	function formatCatapultObject(payload, type) {
		return !Array.isArray(payload)
			? modelFormatter[type].format(payload)
			: formatArray(modelFormatter[type], payload);
	}

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
}

export default {
	/**
	 * Creates server formatters around a model formatter.
	 * @param {object} modelFormatter The model formatter.
	 * @returns {object} The server formatters.
	 */
	create: modelFormatter => ({
		/**
		 * Restify compatible formatter for JSON responses.
		 * @param {object} req The request.
		 * @param {object} res The response.
		 * @param {object} body The body.
		 * @param {Function} cb The callback.
		 * @returns {object} The result of the callback.
		 */
		json: (req, res, body, cb) => {
			// implementation based on https://github.com/restify/node-restify/blob/4.x/lib/formatters/json.js
			const view = formatBody(modelFormatter, body);
			if (view.statusCode)
				res.statusCode = view.statusCode;

			res.setHeader('Content-Length', Buffer.byteLength(view.json));
			return cb(null, view.json);
		},

		/**
		 * Websocket formatter.
		 * @param {object} body The body.
		 * @returns {object} The formatted body.
		 */
		ws: body => (isCatapultObject(body) && 'raw' === body.type ? body.payload : formatBody(modelFormatter, body).json)
	})
};

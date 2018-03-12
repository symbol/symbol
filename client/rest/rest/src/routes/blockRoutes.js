const dbFacade = require('./dbFacade');
const routeResultTypes = require('./routeResultTypes');
const routeUtils = require('./routeUtils');
const errors = require('../server/errors');

const parseHeight = params => routeUtils.parseArgument(params, 'height', 'uint');

const getLimit = (validLimits, params) => {
	const limit = routeUtils.parseArgument(params, 'limit', 'uint');
	return -1 === validLimits.indexOf(limit) ? undefined : limit;
};

const alignDown = (height, alignment) => (Math.floor((height - 1) / alignment) * alignment) + 1;

module.exports = {
	register: (server, db, { config }) => {
		const validPageSizes = routeUtils.generateValidPageSizes(config.pageSize); // throws if there is not at least one valid page size

		server.get('/blocks/:height/limit/:limit', (req, res, next) => {
			const height = parseHeight(req.params);
			const limit = getLimit(validPageSizes, req.params);

			const sanitizedLimit = limit || validPageSizes[0];
			const sanitizedHeight = alignDown(height || 1, sanitizedLimit);
			if (sanitizedHeight !== height || !limit)
				return res.redirect(`/blocks/${sanitizedHeight}/limit/${sanitizedLimit}`, next); // redirect calls next

			return db.blocksFrom(height, limit).then(blocks => {
				res.send({ payload: blocks, type: routeResultTypes.block });
				next();
			});
		});

		server.get('/block/:height', (req, res, next) => {
			const height = parseHeight(req.params);

			return dbFacade.runHeightDependentOperation(db, height, () => db.blockAtHeight(height))
				.then(result => result.payload)
				.then(routeUtils.createSender(routeResultTypes.block).sendOne(height, res, next));
		});

		server.get('/block/:height/transactions', (req, res, next) => {
			const height = parseHeight(req.params);
			const pagingOptions = routeUtils.parsePagingArguments(req.params);

			const operation = () => db.transactionsAtHeight(height, pagingOptions.id, pagingOptions.pageSize);
			return dbFacade.runHeightDependentOperation(db, height, operation)
				.then(result => {
					if (!result.isRequestValid) {
						res.send(errors.createNotFoundError(height));
						return next();
					}

					return routeUtils.createSender(routeResultTypes.transfer).sendArray('height', res, next)(result.payload);
				});
		});
	}
};

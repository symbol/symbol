import errors from '../server/errors';
import routeResultTypes from './routeResultTypes';
import routeUtils from './routeUtils';

function parseHeight(params) {
	return routeUtils.parseArgument(params, 'height', 'uint');
}

function getGrouping(params) {
	const Valid_Groupings = [25, 50, 75, 100];
	const grouping = routeUtils.parseArgument(params, 'grouping', 'uint');
	return -1 === Valid_Groupings.indexOf(grouping) ? undefined : grouping;
}

function alignDown(height, alignment) {
	return (Math.floor((height - 1) / alignment) * alignment) + 1;
}

export default {
	register: (server, db, services) => {
		server.get('/blocks/from/:height/group/:grouping', (req, res, next) => {
			const height = parseHeight(req.params);
			const grouping = getGrouping(req.params);
			const sanitizedGrouping = grouping || 25;
			const sanitizedHeight = alignDown(height || 1, sanitizedGrouping);
			if (sanitizedHeight !== height || !grouping) {
				res.redirect(`/blocks/from/${sanitizedHeight}/group/${sanitizedGrouping}`, next);
				next();
				return undefined;
			}

			return db.blocksFrom(height, grouping).then(blocks => {
				res.send({ payload: blocks, type: routeResultTypes.block });
				next();
			});
		});

		server.get('/block/height/:height', (req, res, next) => {
			const height = parseHeight(req.params);

			const chainInfoPromise = db.chainInfo();
			const blockPromise = db.blockAtHeight(height);
			return Promise.all([chainInfoPromise, blockPromise])
				.then(results => {
					const chainInfo = results[0];
					const block = results[1];

					if (!block || height > chainInfo.height)
						res.send(errors.createNotFoundError(height));
					else
						res.send({ payload: block, type: routeResultTypes.block });

					next();
				});
		});

		server.ws('/ws/block', client => {
			services.entityEmitter.on('block', block => {
				client.send({ payload: block, type: routeResultTypes.block });
			});
		});

		server.get('/block/height/:height/transactions', (req, res, next) => {
			const height = parseHeight(req.params);
			const pagingOptions = routeUtils.parsePagingArguments(req.params);

			const chainInfoPromise = db.chainInfo();
			const transactionsPromise = db.transactionsAtHeight(height, pagingOptions.id, pagingOptions.pageSize);
			const sendTransactions = routeUtils.sendEntities('height', routeResultTypes.transfer, res, next);

			return Promise.all([chainInfoPromise, transactionsPromise])
				.then(results => {
					const chainInfo = results[0];
					const transactions = results[1];

					if (height > chainInfo.height) {
						res.send(errors.createNotFoundError(height));
						next();
					} else {
						sendTransactions(transactions);
					}
				});
		});
	}
};

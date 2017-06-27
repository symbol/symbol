import routeResultTypes from './routeResultTypes';
import routeUtils from './routeUtils';

export default {
	register: (server, db) => {
		server.get('/diagnostic/storage', (req, res, next) =>
			db.storageInfo().then(storageInfo => {
				res.send({ payload: storageInfo, type: routeResultTypes.storageInfo });
				next();
			}));

		server.get('/diagnostic/blocks/:height/count/:count', (req, res, next) => {
			const parseUint = paramName => routeUtils.parseArgument(req.params, paramName, 'uint');
			const height = parseUint('height');
			const count = parseUint('count');

			return db.blocksFrom(height, count).then(blocks => {
				res.send({ payload: blocks, type: routeResultTypes.block });
				next();
			});
		});
	}
};

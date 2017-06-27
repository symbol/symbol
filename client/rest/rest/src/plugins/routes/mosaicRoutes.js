import catapult from 'catapult-sdk';
import routeUtils from '../../routes/routeUtils';

const uint64 = catapult.utils.uint64;

export default {
	register: (server, db) => {
		function sendMosaicOrNotFound(id, res, next) {
			return routeUtils.sendEntityOrNotFound(id, 'mosaicDescriptor', res, next);
		}

		function sendMosaics(id, res, next) {
			return routeUtils.sendEntities(id, 'mosaicDescriptor', res, next);
		}

		server.get('/mosaic/id/:id', (req, res, next) => {
			const id = routeUtils.parseArgument(req.params, 'id', uint64.fromHex);
			return db.mosaicById(id)
				.then(sendMosaicOrNotFound(req.params.id, res, next));
		});

		server.post('/mosaics/ids', (req, res, next) => {
			const ids = routeUtils.parseArgumentAsArray(req.params, 'ids', uint64.fromHex);
			return db.mosaicsByIds(ids)
				.then(sendMosaics('ids', res, next));
		});

		server.get('/namespace/:namespaceId/mosaics', (req, res, next) => {
			const id = routeUtils.parseArgument(req.params, 'namespaceId', uint64.fromHex);
			const pagingOptions = routeUtils.parsePagingArguments(req.params);

			return db.mosaicsByNamespaceId(id, pagingOptions.id, pagingOptions.pageSize)
				.then(sendMosaics('namespaceId', res, next));
		});

		server.post('/names/mosaic/ids', (req, res, next) => {
			const ids = routeUtils.parseArgumentAsArray(req.params, 'ids', uint64.fromHex);
			const type = catapult.model.EntityType.mosaicDefinition;
			return db.catapultDb
				.findNamesByIds(ids, type, { id: 'mosaicId', name: 'name', parentId: 'parentId' })
				.then(routeUtils.sendEntities('ids', 'mosaicNameTuple', res, next));
		});
	}
};

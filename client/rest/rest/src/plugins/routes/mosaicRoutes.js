const catapult = require('catapult-sdk');
const routeUtils = require('../../routes/routeUtils');

const { uint64 } = catapult.utils;

module.exports = {
	register: (server, db) => {
		const mosaicSender = routeUtils.createSender('mosaicDescriptor');

		routeUtils.addGetPostDocumentRoutes(
			server,
			mosaicSender,
			{ base: '/mosaic', singular: 'mosaicId', plural: 'mosaicIds' },
			params => db.mosaicsByIds(params),
			uint64.fromHex
		);

		server.get('/namespace/:namespaceId/mosaics', (req, res, next) => {
			const namespaceId = routeUtils.parseArgument(req.params, 'namespaceId', uint64.fromHex);
			const pagingOptions = routeUtils.parsePagingArguments(req.params);

			return db.mosaicsByNamespaceId(namespaceId, pagingOptions.id, pagingOptions.pageSize)
				.then(mosaicSender.sendArray('namespaceId', res, next));
		});

		server.post('/mosaic/names', (req, res, next) => {
			const mosaicIds = routeUtils.parseArgumentAsArray(req.params, 'mosaicIds', uint64.fromHex);
			const type = catapult.model.EntityType.mosaicDefinition;
			return db.catapultDb
				.findNamesByIds(mosaicIds, type, { id: 'mosaicId', name: 'name', parentId: 'parentId' })
				.then(routeUtils.createSender('mosaicNameTuple').sendArray('mosaicIds', res, next));
		});
	}
};

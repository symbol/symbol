import catapult from 'catapult-sdk';
import routeUtils from '../../routes/routeUtils';

const uint64 = catapult.utils.uint64;
const convert = catapult.utils.convert;

export default {
	register: (server, db) => {
		function sendNamespaceOrNotFound(id, res, next) {
			return routeUtils.sendEntityOrNotFound(id, 'namespaceDescriptor', res, next);
		}

		function sendNamespaces(id, res, next) {
			return routeUtils.sendEntities(id, 'namespaceDescriptor', res, next);
		}

		server.get('/namespace/id/:id', (req, res, next) => {
			const id = routeUtils.parseArgument(req.params, 'id', uint64.fromHex);
			return db.namespaceById(id)
				.then(sendNamespaceOrNotFound(req.params.id, res, next));
		});

		server.get('/account/key/:publicKey/namespaces', (req, res, next) => {
			const publicKey = routeUtils.parseArgument(req.params, 'publicKey', convert.hexToUint8);
			const pagingOptions = routeUtils.parsePagingArguments(req.params);
			return db.namespacesByOwner(publicKey, pagingOptions.id, pagingOptions.pageSize)
				.then(sendNamespaces('publicKey', res, next));
		});

		function collectNames(namespaceNameTuples, ids) {
			const type = catapult.model.EntityType.registerNamespace;
			return db.catapultDb.findNamesByIds(ids, type, { id: 'namespaceId', name: 'name', parentId: 'parentId' })
				.then(tuples => {
					for (const nameTuple of tuples)
						namespaceNameTuples.push(nameTuple);

					return tuples
						.filter(nameTuple => !nameTuple.parentId.isZero())
						.map(nameTuple => nameTuple.parentId);
				});
		}

		server.post('/names/namespace/ids', (req, res, next) => {
			const ids = routeUtils.parseArgumentAsArray(req.params, 'ids', uint64.fromHex);
			return new Promise(resolve => {
				const namespaceNameTuples = [];
				function chain(nextIds) {
					if (0 === nextIds.length)
						resolve(namespaceNameTuples);
					else
						collectNames(namespaceNameTuples, nextIds).then(chain);
				}

				collectNames(namespaceNameTuples, ids).then(chain);
			})
			.then(routeUtils.sendEntities('ids', 'namespaceNameTuple', res, next));
		});
	}
};

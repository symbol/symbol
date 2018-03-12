const AccountType = require('../AccountType');
const catapult = require('catapult-sdk');
const routeUtils = require('../../routes/routeUtils');

const { uint64 } = catapult.utils;

module.exports = {
	register: (server, db) => {
		const namespaceSender = routeUtils.createSender('namespaceDescriptor');

		server.get('/namespace/:namespaceId', (req, res, next) => {
			const namespaceId = routeUtils.parseArgument(req.params, 'namespaceId', uint64.fromHex);
			return db.namespaceById(namespaceId)
				.then(namespaceSender.sendOne(req.params.namespaceId, res, next));
		});

		server.get('/account/:accountId/namespaces', (req, res, next) => {
			const [type, accountId] = routeUtils.parseArgument(req.params, 'accountId', 'accountId');
			const pagingOptions = routeUtils.parsePagingArguments(req.params);

			return db.namespacesByOwners(type, [accountId], pagingOptions.id, pagingOptions.pageSize)
				.then(namespaceSender.sendArray('accountId', res, next));
		});

		server.post('/account/namespaces', (req, res, next) => {
			const idOptions = Array.isArray(req.params.publicKeys)
				? { keyName: 'publicKeys', parserName: 'publicKey', type: AccountType.publicKey }
				: { keyName: 'addresses', parserName: 'address', type: AccountType.address };

			const accountIds = routeUtils.parseArgumentAsArray(req.params, idOptions.keyName, idOptions.parserName);
			const pagingOptions = routeUtils.parsePagingArguments(req.params);
			return db.namespacesByOwners(idOptions.type, accountIds, pagingOptions.id, pagingOptions.pageSize)
				.then(namespaceSender.sendArray(idOptions.keyName, res, next));
		});

		const collectNames = (namespaceNameTuples, namespaceIds) => {
			const type = catapult.model.EntityType.registerNamespace;
			return db.catapultDb.findNamesByIds(namespaceIds, type, { id: 'namespaceId', name: 'name', parentId: 'parentId' })
				.then(nameTuples => {
					nameTuples.forEach(nameTuple => {
						// db returns null instead of undefined when parentId is not present
						if (null === nameTuple.parentId)
							delete nameTuple.parentId;

						namespaceNameTuples.push(nameTuple);
					});

					// process all parent namespaces next
					return nameTuples
						.filter(nameTuple => undefined !== nameTuple.parentId)
						.map(nameTuple => nameTuple.parentId);
				});
		};

		server.post('/namespace/names', (req, res, next) => {
			const namespaceIds = routeUtils.parseArgumentAsArray(req.params, 'namespaceIds', uint64.fromHex);
			const nameTuplesFuture = new Promise(resolve => {
				const namespaceNameTuples = [];
				const chain = nextIds => {
					if (0 === nextIds.length)
						resolve(namespaceNameTuples);
					else
						collectNames(namespaceNameTuples, nextIds).then(chain);
				};

				collectNames(namespaceNameTuples, namespaceIds).then(chain);
			});

			return nameTuplesFuture.then(routeUtils.createSender('namespaceNameTuple').sendArray('namespaceIds', res, next));
		});
	}
};

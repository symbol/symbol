const MongoDb = require('mongodb');
const NamespaceDb = require('../../../../src/plugins/db/NamespaceDb');
const test = require('../../../testUtils');
const dbTestUtils = require('../../../db/utils/dbTestUtils');

const { Binary, Long } = MongoDb;

const Namespace_Types = { root: 0, child: 1 };

const createNamespace = (id, owner, namespaceType, parentIdOrDuration, path, lifetime, active) => {
	// namespace data
	const namespace = {
		owner: new Binary(owner.publicKey),
		ownerAddress: new Binary(owner.address),
		startHeight: Long.fromNumber(lifetime.start),
		endHeight: Long.fromNumber(lifetime.end),
		depth: path.length,
		level0: Long.fromNumber(path[0]),
		type: namespaceType,
		name: namespaceType === Namespace_Types.root ? `root${id}` : `child${id}`
	};

	if (1 < path.length)
		namespace.level1 = Long.fromNumber(path[1]);

	if (2 < path.length)
		namespace.level2 = Long.fromNumber(path[2]);

	if (namespaceType === Namespace_Types.root)
		namespace.duration = Long.fromNumber(parentIdOrDuration);
	else
		namespace.parentId = Long.fromNumber(parentIdOrDuration);

	return { _id: dbTestUtils.db.createObjectId(id), meta: { active }, namespace };
};

const createNamespaces = (numRounds, owner, startdId = 0) => {
	// the depth is determined by id % 3:
	// 0: root
	// 1: child of depth 1
	// 2: child of depth 2
	let id = startdId;
	const namespaces = [];

	const push = (namespaceOwner, i, active) => {
		const namespaceType = 0 === i % 3 ? Namespace_Types.root : Namespace_Types.child;
		const lifetime = { start: 10 * i, end: 10 * (i + 1) };
		// use a 12300 base for namespace id, to distinguish from id inside the tests
		let nsId = 12300 + i;
		const path = [nsId];

		while (0 !== nsId % 3)
			path.unshift(--nsId);

		namespaces.push(createNamespace(id++, namespaceOwner, namespaceType, id, path, lifetime, active));
	};

	for (let i = 0; i < numRounds; ++i)
		push(owner, i, true);

	for (let i = 0; i < numRounds; ++i)
		push(owner, i, false);

	return namespaces;
};

const namespaceDbTestUtils = {
	db: {
		createNamespace,
		createNamespaces,
		runDbTest: (dbEntities, issueDbCommand, assertDbCommandResult) =>
			dbTestUtils.db.runDbTest(dbEntities, 'namespaces', db => new NamespaceDb(db), issueDbCommand, assertDbCommandResult)
	}
};
Object.assign(namespaceDbTestUtils, test);

module.exports = namespaceDbTestUtils;

/*
 * Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
 * Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
 * All rights reserved.
 *
 * This file is part of Catapult.
 *
 * Catapult is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Catapult is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Catapult.  If not, see <http://www.gnu.org/licenses/>.
 */

const NamespaceDb = require('../../../src/plugins/namespace/NamespaceDb');
const dbTestUtils = require('../../db/utils/dbTestUtils');
const test = require('../../testUtils');
const MongoDb = require('mongodb');

const { Binary, Long } = MongoDb;

const Namespace_Types = { root: 0, child: 1 };

const createAlias = id => {
	switch (id % 3) {
	case 1: return { type: 1, mosaicId: 1000 };
	case 2: return { type: 2, address: new Binary(dbTestUtils.random.address()) };
	default: return { type: 0 };
	}
};

const createNamespace = (id, owner, namespaceType, parentIdOrDuration, path, lifetime, active, alias) => {
	// namespace data
	const namespace = {
		ownerAddress: new Binary(owner.address),
		startHeight: Long.fromNumber(lifetime.start),
		endHeight: Long.fromNumber(lifetime.end),
		depth: path.length,
		level0: Long.fromNumber(path[0]),
		type: namespaceType,
		name: namespaceType === Namespace_Types.root ? `root${id}` : `child${id}`,
		alias
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

		namespaces.push(createNamespace(id++, namespaceOwner, namespaceType, id, path, lifetime, active, createAlias(i)));
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

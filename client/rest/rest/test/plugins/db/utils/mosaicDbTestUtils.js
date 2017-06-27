import MongoDb from 'mongodb';
import NamespaceDb from '../../../../src/plugins/db/NamespaceDb';
import test from '../../../testUtils';
import dbTestUtils from '../../../db/utils/dbTestUtils';

const Binary = MongoDb.Binary;
const Long = MongoDb.Long;

function createMosaic(id, mosaicId, owner, parentId, active) {
	// mosaic data
	const mosaic = {
		owner: new Binary(owner),
		mosaicId: Long.fromNumber(mosaicId),
		namespaceId: Long.fromNumber(parentId)
	};

	return { _id: dbTestUtils.db.createObjectId(id), mosaic, meta: { active } };
}

function createMosaics(owner, numNamespaces, numMosaicsPerNamespace) {
	// mosaic ids start at 10000, namespace ids start at 20000 in order to differentiate from db _id
	// there is only 1 inactive mosaic per namespace and it has the same id as smallest active mosaic in each namespace
	const mosaics = [];
	let dbId = 0;
	let mosaicId = 10000;
	for (let namespaceId = 0; namespaceId < numNamespaces; ++namespaceId) {
		for (let i = 0; i < numMosaicsPerNamespace; ++i)
			mosaics.push(createMosaic(dbId++, mosaicId++, owner, 20000 + namespaceId, true));
	}

	// in every namespace add a single inactive mosaic
	mosaicId = 10000;
	for (let namespaceId = 0; namespaceId < numNamespaces; ++namespaceId) {
		mosaics.push(createMosaic(dbId++, mosaicId, owner, 20000 + namespaceId, false));
		mosaicId += numMosaicsPerNamespace;
	}

	return mosaics;
}

const mosaicDbTestUtils = {
	db: {
		createMosaic,
		createMosaics,
		runDbTest: (dbEntities, issueDbCommand, assertDbCommandResult) =>
			dbTestUtils.db.runDbTest(dbEntities, 'mosaics', db => new NamespaceDb(db), issueDbCommand, assertDbCommandResult)
	}
};
Object.assign(mosaicDbTestUtils, test);

export default mosaicDbTestUtils;

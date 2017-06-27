import MongoDb from 'mongodb';

const Long = MongoDb.Long;

function createActiveConditions() {
	const conditions = { $and: [{ 'meta.active': true }] };
	return conditions;
}

export default class NamespaceDb {

	/**
	 * Creates NamespaceDb around CatapultDb.
	 * @param {module:db/CatapultDb} db Catapult db instance.
	 */
	constructor(db) {
		this.catapultDb = db;
	}

	// region namespace retrieval

	/**
	 * Retrieves a namespace.
	 * @param {module:catapult.utils/uint64~uint64} id The namespace id.
	 * @returns {Promise.<object>} The namespace.
	 */
	namespaceById(id) {
		const namespaceId = new Long(id[0], id[1]);
		const conditions = { $or: [] };

		for (let level = 0; 3 > level; ++level) {
			const conjunction = createActiveConditions();
			conjunction.$and.push({ [`namespace.level${level}`]: namespaceId });
			conjunction.$and.push({ 'namespace.depth': level + 1 });

			conditions.$or.push(conjunction);
		}

		return this.catapultDb.queryDocument('namespaces', conditions)
			.then(this.catapultDb.sanitizer.copyAndDeleteId);
	}

	/**
	 * Retrieves namespaces owned by given owner.
	 * @param {string} publicKey The owner's public key.
	 * @param {string} id Paging id.
	 * @param {int} pageSize Page size.
	 * @param {object} options Additional options.
	 * @returns {Promise.<array>} Owned namespaces.
	 */
	namespacesByOwner(publicKey, id, pageSize, options) {
		const bufferPublicKey = Buffer.from(publicKey);
		const conditions = createActiveConditions();
		conditions.$and.push({ 'namespace.owner': bufferPublicKey });
		return this.catapultDb.queryPagedDocuments('namespaces', conditions, id, pageSize, options)
			.then(this.catapultDb.sanitizer.copyAndDeleteIds);
	}

	// endregion

	// region mosaic retrieval

	/**
	 * Retrieves a mosaic.
	 * @param {module:catapult.utils/uint64~uint64} id The mosaic id.
	 * @returns {Promise.<object>} The mosaic.
	 */
	mosaicById(id) {
		const mosaicId = new Long(id[0], id[1]);
		const conditions = createActiveConditions();
		conditions.$and.push({ 'mosaic.mosaicId': mosaicId });
		return this.catapultDb.queryDocument('mosaics', conditions)
			.then(this.catapultDb.sanitizer.copyAndDeleteId);
	}

	/**
	 * Retrieves mosaics.
	 * @param {Array.<module:catapult.utils/uint64~uint64>} ids The mosaic ids.
	 * @returns {Promise.<array>} The mosaics.
	 */
	mosaicsByIds(ids) {
		const mosaicIds = ids.map(id => new Long(id[0], id[1]));
		const conditions = createActiveConditions();
		conditions.$and.push({ 'mosaic.mosaicId': { $in: mosaicIds } });
		const collection = this.catapultDb.database.collection('mosaics');
		return collection.find(conditions)
			.sort({ _id: -1 })
			.toArray()
			.then(entities => Promise.resolve(this.catapultDb.sanitizer.copyAndDeleteIds(entities)));
	}

	/**
	 * Retrieves mosaics under given namespace.
	 * @param {module:catapult.utils/uint64~uint64} namespaceId The namespace id.
	 * @param {string} id Paging id.
	 * @param {int} pageSize Page size.
	 * @param {object} options Additional options.
	 * @returns {Promise.<array>} The mosaics.
	 */
	mosaicsByNamespaceId(namespaceId, id, pageSize, options) {
		const namespaceIdLong = new Long(namespaceId[0], namespaceId[1]);
		const conditions = createActiveConditions();
		conditions.$and.push({ 'mosaic.namespaceId': namespaceIdLong });
		return this.catapultDb.queryPagedDocuments('mosaics', conditions, id, pageSize, options)
			.then(this.catapultDb.sanitizer.copyAndDeleteIds);
	}

	// endregion
}

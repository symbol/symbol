import MongoDb from 'mongodb';
import winston from 'winston';
import cursorUtils from './cursorUtils';

const connector = {
	connectToDatabase(url, dbName) {
		const connectionString = `${url}${dbName}`;
		return MongoDb.MongoClient.connect(connectionString, { promoteLongs: false })
			.then(db => {
				winston.verbose(`connected to mongo at ${connectionString}`);
				return db;
			});
	},

	/**
	 * Starts tailing a mongo db oplog.
	 * @param {string} url The database url.
	 * @param {object} query The database query.
	 * @returns {Promise}
	 * A promise that is resolved with an emitter and a db connection after the tail has been started.
	 */
	startTailingOplog(url, query) {
		return connector.connectToDatabase(url, 'local')
			.then(db => ({ db, emitter: cursorUtils.createTailDataEmitter(db.collection('oplog.rs'), query) }))
			.then(emitterConnection => emitterConnection.emitter.start().then(() => emitterConnection));
	}
};

export default connector;

const MongoDb = require('mongodb');
const winston = require('winston');

const connector = {
	connectToDatabase(url, dbName) {
		const connectionString = `${url}${dbName}`;
		return MongoDb.MongoClient.connect(connectionString, { promoteLongs: false })
			.then(client => {
				winston.verbose(`connected to mongo at ${connectionString}`);
				return client;
			});
	}
};

module.exports = connector;

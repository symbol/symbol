const parseArgs = require('minimist');
const catapult = require('catapult-sdk');

module.exports = {
	url: (() => {
		const args = parseArgs(process.argv.slice(2));
		const mongoHost = args.mongoHost || '127.0.0.1';
		return `mongodb://${mongoHost}:27017/`;
	})(),
	networkId: catapult.model.networkInfo.networks.mijinTest.id
};

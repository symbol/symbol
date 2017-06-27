import parseArgs from 'minimist';
import catapult from 'catapult-sdk';

export default {
	url: (function () {
		const args = parseArgs(process.argv.slice(2));
		const mongoPort = args.mongoPort || 27017;
		return `mongodb://localhost:${mongoPort}/`;
	})(),
	networkId: catapult.model.networkInfo.networks.mijinTest.id
};

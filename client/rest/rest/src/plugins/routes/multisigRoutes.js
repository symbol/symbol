import catapult from 'catapult-sdk';
import routeUtils from '../../routes/routeUtils';

const convert = catapult.utils.convert;

export default {
	register: (server, db) => {
		server.get('/account/key/:publicKey/multisig', (req, res, next) => {
			const publicKey = routeUtils.parseArgument(req.params, 'publicKey', convert.hexToUint8);
			return db.multisigByAccount(publicKey)
				.then(routeUtils.sendEntityOrNotFound(req.params.publicKey, 'multisigEntry', res, next));
		});
	}
};

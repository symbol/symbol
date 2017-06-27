import catapult from 'catapult-sdk';
import routeResultTypes from './routeResultTypes';
import routeUtils from './routeUtils';

const convert = catapult.utils.convert;
const packetHeader = catapult.packet.header;
const PacketType = catapult.packet.PacketType;

function createPushTransactionsPacketBuffer(hexData) {
	const data = convert.hexToUint8(hexData);
	const length = packetHeader.size + data.length;
	const header = packetHeader.createBuffer(PacketType.pushTransactions, length);
	const buffers = [header, Buffer.from(data)];
	return Buffer.concat(buffers, length);
}

export default {
	register: (server, db, services) => {
		function sendTransactionOrNotFound(id, res, next) {
			return routeUtils.sendEntityOrNotFound(id, routeResultTypes.transfer, res, next);
		}

		server.put('/transaction/send', (req, res, next) => {
			const packetBuffer = routeUtils.parseArgument(req.params, 'payload', createPushTransactionsPacketBuffer);
			return services.connections.lease()
				.then(connection => connection.send(packetBuffer))
				.then(() => {
					res.send(202, { message: 'transaction(s) were pushed to the network' });
					next();
				});
		});

		server.get('/transaction/id/:id', (req, res, next) => {
			const id = routeUtils.parseArgument(req.params, 'id', 'objectId');
			return db.transactionById(id)
				.then(sendTransactionOrNotFound(id, res, next));
		});
	}
};

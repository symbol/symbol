const aggregateRoutes = require('../../../src/plugins/routes/aggregateRoutes');
const test = require('../../routes/utils/routeTestUtils');

describe('aggregate routes', () => {
	describe('PUT transaction partial', () => {
		test.route.packet.addPutPacketRouteTests(aggregateRoutes.register, {
			routeName: '/transaction/partial',
			packetType: '500',
			inputs: {
				valid: {
					params: { payload: '123456' },
					parsed: Buffer.of(
						0x0B, 0x00, 0x00, 0x00, // size (header)
						0xF4, 0x01, 0x00, 0x00, // type (header)
						0x12, 0x34, 0x56 // payload
					)
				},
				invalid: {
					params: { payload: '1234S6' },
					error: { key: 'payload' }
				}
			}
		});
	});

	describe('PUT transaction cosignature', () => {
		test.route.packet.addPutPacketRouteTests(aggregateRoutes.register, {
			routeName: '/transaction/cosignature',
			packetType: '501',
			inputs: {
				valid: {
					params: { signer: '123456', signature: '998811', parentHash: 'ABEF' },
					parsed: Buffer.of(
						0x10, 0x00, 0x00, 0x00, // size (header)
						0xF5, 0x01, 0x00, 0x00, // type (header)
						0x12, 0x34, 0x56, 0x99, 0x88, 0x11, 0xAB, 0xEF // payload (signer, signature, parentHash)
					)
				},
				invalid: {
					params: { signer: '123456', signature: '998S11', parentHash: 'ABEF' },
					error: { key: 'signature' }
				}
			}
		});
	});
});

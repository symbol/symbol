import {expect} from 'chai';
import {convertPing} from '../src/convert.js'

describe('convert', () => {
	describe('ping', () => {
		it('can convert a ping packet to json', () => {
			const buffer = new Buffer('000000000200000045A30000', 'hex')

			const json = convertPing(buffer);

			expect(json).to.deep.equal({ 'type': 0x0002, 'heartbeat': 0xA345 });
		});
	});
});

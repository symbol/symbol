/*
 * Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
 * Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
 * All rights reserved.
 *
 * This file is part of Catapult.
 *
 * Catapult is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Catapult is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Catapult.  If not, see <http://www.gnu.org/licenses/>.
 */

const stateTreesCodec = require('../../src/sockets/stateTreesCodec');
const catapult = require('catapult-sdk');
const { expect } = require('chai');

const { BinaryParser } = catapult.parser;
const { convert } = catapult.utils;

describe('deserialize', () => {
	it('returns a deserialized state tree object', () => {
		// Arrange:
		const tree = [
			'9922093F19F7160BDCBCA8AA48499DA8DF532D4102745670B85AA4BDF63B8D59',
			'E8FCFD95CA220D442BE748F5494001A682DC8015A152EBC433222136E99A96B8',
			'C1C1062C63CAB4197C87B366052ECE3F4FEAE575D81A7F728F4E3704613AF876',
			'F8E8FCDAD1B94D2C76D769B113FF5CAC5D5170772F2D80E466EB04FCA23D6887',
			'2D3418274BBC250616223C162534B460216AED82C4FA9A87B53083B7BA7A9391',
			'AEAF30ED55BBE4805C53E5232D88242F0CF719F99A8E6D339BCBF5D5DE85E1FB',
			'AFE6C917BABA60ADC1512040CC35033B563DAFD1718FA486AB1F3D9B84866B27'
		].map((treeNode => Buffer.from(convert.hexToUint8(treeNode))));

		const binaryParser = new BinaryParser();
		binaryParser.push(Buffer.concat(tree));

		// Assert:
		expect(stateTreesCodec.deserialize(binaryParser)).to.deep.equal({ tree });
	});
});

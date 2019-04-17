/*
 * Copyright (c) 2016-present,
 * Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

const ModelType = require('../../src/model/ModelType');
const { expect } = require('chai');

describe('model type enumeration', () => {
	it('exposes expected types', () => {
		// Assert:
		expect(ModelType).to.deep.equal({
			none: 0,
			object: 1,
			array: 2,
			dictionary: 3,
			binary: 4,
			uint64: 5,
			objectId: 6,
			string: 7,
			statusCode: 8,
			uint16: 9,
			max: 9
		});
	});
});

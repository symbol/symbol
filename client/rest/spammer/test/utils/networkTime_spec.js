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

const networkTime = require('../../src/utils/networkTime');
const { expect } = require('chai');

describe('network time', () => {
	it('is positive', () => {
		// Act:
		const time = networkTime.getNetworkTime();

		// Assert:
		expect(time).to.be.at.least(1);
	});

	it('is increasing', () => {
		// Arrange:
		const time1 = networkTime.getNetworkTime();

		// Act:
		setTimeout(() => {
			const time2 = networkTime.getNetworkTime();

			// Assert:
			expect(time1).to.be.below(time2);
		}, 10);
	});
});

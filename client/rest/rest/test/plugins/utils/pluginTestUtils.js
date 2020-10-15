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

const { expect } = require('chai');

const wrapCreateDbTest = (resultName, action) => {
	describe('create db', () => {
		it(`returns ${resultName}`, action);
	});
};

module.exports = {
	assertThat: {
		pluginDoesNotCreateDb: plugin => {
			wrapCreateDbTest('undefined', () => {
				// Act:
				const db = plugin.createDb();

				// Assert:
				expect(db).to.equal(undefined);
			});
		},

		pluginCreatesDb: (plugin, expectedDbType) => {
			wrapCreateDbTest('db', () => {
				// Act:
				const db = plugin.createDb();

				// Assert:
				expect(db).to.be.instanceOf(expectedDbType);
			});
		},

		pluginDoesNotRegisterAdditionalTransactionStates: plugin => {
			describe('register transaction states', () => {
				it('does not register states', () => {
					// Arrange:
					const states = [];

					// Act:
					plugin.registerTransactionStates(states);

					// Assert:
					expect(states.length).to.equal(0);
				});
			});
		},

		pluginDoesNotRegisterAdditionalMessageChannels: plugin => {
			describe('register message channels', () => {
				it('does not register channels', () => {
					// Arrange:
					let numAddCalls = 0;
					const builder = { add: () => { ++numAddCalls; } };

					// Act:
					plugin.registerMessageChannels(builder);

					// Assert:
					expect(numAddCalls).to.equal(0);
				});
			});
		}
	}
};

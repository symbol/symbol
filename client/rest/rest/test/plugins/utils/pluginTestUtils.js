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

const lockRoutes = require('../../../src/plugins/routes/lockRoutes');
const routeAccountIdGetTestUtils = require('./utils/routeAccountIdGetTestUtils');

describe('lock routes', () => {
	describe('get hash lock infos by account', () => {
		routeAccountIdGetTestUtils.addDefaultTests({
			registerRoutes: lockRoutes.register,
			route: '/account/:accountId/lock/hash',
			dbApiName: 'hashLocksByAccounts',
			dbType: 'hashLockInfo'
		});
	});

	describe('get secret lock infos by account', () => {
		routeAccountIdGetTestUtils.addDefaultTests({
			registerRoutes: lockRoutes.register,
			route: '/account/:accountId/lock/secret',
			dbApiName: 'secretLocksByAccounts',
			dbType: 'secretLockInfo'
		});
	});
});

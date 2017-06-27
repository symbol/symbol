import accountRoutes from './accountRoutes';
import blockRoutes from './blockRoutes';
import chainRoutes from './chainRoutes';
import diagnosticRoutes from './diagnosticRoutes';
import transactionRoutes from './transactionRoutes';

export default {
	register: (...args) => {
		for (const routes of [accountRoutes, blockRoutes, chainRoutes, diagnosticRoutes, transactionRoutes])
			routes.register(...args);
	}
};

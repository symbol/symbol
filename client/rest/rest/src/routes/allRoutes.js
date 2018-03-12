const accountRoutes = require('./accountRoutes');
const blockRoutes = require('./blockRoutes');
const chainRoutes = require('./chainRoutes');
const diagnosticRoutes = require('./diagnosticRoutes');
const networkRoutes = require('./networkRoutes');
const transactionRoutes = require('./transactionRoutes');
const transactionStatusRoutes = require('./transactionStatusRoutes');
const wsRoutes = require('./wsRoutes');

module.exports = {
	register: (...args) => {
		const allRoutes = [
			accountRoutes,
			blockRoutes,
			chainRoutes,
			diagnosticRoutes,
			networkRoutes,
			transactionRoutes,
			transactionStatusRoutes,
			wsRoutes];
		allRoutes.forEach(routes => {
			routes.register(...args);
		});
	}
};

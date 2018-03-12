const AccountType = require('../AccountType');
const routeUtils = require('../../routes/routeUtils');

module.exports = {
	register: (server, db) => {
		server.get('/account/:accountId/multisig', (req, res, next) => {
			const [type, accountId] = routeUtils.parseArgument(req.params, 'accountId', 'accountId');

			return db.multisigsByAccounts(type, [accountId])
				.then(routeUtils.createSender('multisigEntry').sendOne(req.params.accountId, res, next));
		});

		const getMultisigEntries = (multisigEntries, fieldName) => {
			const publicKeys = new Set();
			multisigEntries.forEach(multisigEntry => multisigEntry.multisig[fieldName].forEach(publicKey => {
				publicKeys.add(publicKey.buffer);
			}));

			return db.multisigsByAccounts(AccountType.publicKey, Array.from(publicKeys));
		};

		server.get('/account/:accountId/multisig/graph', (req, res, next) => {
			const [type, accountId] = routeUtils.parseArgument(req.params, 'accountId', 'accountId');

			const multisigLevels = [];
			return db.multisigsByAccounts(type, [accountId])
				.then(multisigEntries => {
					if (0 === multisigEntries.length)
						return Promise.resolve(undefined);

					multisigLevels.push({
						level: 0,
						multisigEntries: [multisigEntries[0]]
					});

					return Promise.resolve(multisigEntries[0]);
				})
				.then(multisigEntry => {
					if (undefined === multisigEntry)
						return Promise.resolve(undefined);

					const handleUpstream = (level, multisigEntries) => getMultisigEntries(multisigEntries, 'multisigAccounts')
						.then(entries => {
							if (0 === entries.length)
								return Promise.resolve();

							multisigLevels.unshift({ level, multisigEntries: entries });
							return handleUpstream(level - 1, entries);
						});

					const handleDownstream = (level, multisigEntries) => getMultisigEntries(multisigEntries, 'cosignatories')
						.then(entries => {
							if (0 === entries.length)
								return Promise.resolve();

							multisigLevels.push({ level, multisigEntries: entries });
							return handleDownstream(level + 1, entries);
						});

					const upstreamPromise = handleUpstream(-1, [multisigEntry]);
					const downstreamPromise = handleDownstream(1, [multisigEntry]);
					return Promise.all([upstreamPromise, downstreamPromise])
						.then(() => multisigLevels);
				})
				.then(response => {
					const sender = routeUtils.createSender('multisigGraph');
					return undefined === response
						? sender.sendOne(req.params.accountId, res, next)(response)
						: sender.sendArray(req.params.accountId, res, next)(response);
				});
		});
	}
};

export default () => ([
	// root namespace
	{
		type: 'namespace_registration_transaction_v1',
		rentalFeeSink: 'TAMESPACEWH4MKFMBCVFERDPOOP4FK7MTDJEYP35',
		rentalFee: 50000n * 1000000n,
		name: 'roger'
	},

	// child namespace
	{
		type: 'namespace_registration_transaction_v1',
		rentalFeeSink: 'TAMESPACEWH4MKFMBCVFERDPOOP4FK7MTDJEYP35',
		rentalFee: 1n * 1000000n,
		parentName: 'roger',
		name: 'charlie'
	}
]);

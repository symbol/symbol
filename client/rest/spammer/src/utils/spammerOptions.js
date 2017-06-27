import commandLineArgs from 'command-line-args';
import commandLineUsage from 'command-line-usage';

const optionDefinitions = [
	{ name: 'help', alias: 'h', type: Boolean, defaultValue: false },
	{ name: 'predefinedRecipients', alias: 'd', type: Number, defaultValue: 0 },
	{ name: 'address', alias: 'a', type: String, defaultValue: '127.0.0.1' },
	{ name: 'port', alias: 'p', type: Number, defaultValue: 3000 },
	{ name: 'rate', alias: 'r', type: Number, defaultValue: 1 },
	{ name: 'total', alias: 't', type: Number, defaultValue: 10 }
];

const sections = [
	{
		header: 'Catapult spammer',
		content: 'Tool to spam a rest server with random transactions.'
	},
	{
		header: 'Options',
		optionList: [
			{
				name: 'predefinedRecipients',
				alias: 'd',
				description: 'The number of predefined recipients or 0 for random recipients.'
			},
			{
				name: 'address',
				alias: 'a',
				description: 'The host ip address.'
			},
			{
				name: 'port',
				alias: 'p',
				description: 'The port on which to connect.'
			},
			{
				name: 'rate',
				alias: 'r',
				description: 'The desired transaction rate (tx / s).'
			},
			{
				name: 'total',
				alias: 't',
				description: 'The total number of transactions.'
			}
		]
	}
];

export default {
	options: () => commandLineArgs(optionDefinitions),
	usage: () => commandLineUsage(sections)
};

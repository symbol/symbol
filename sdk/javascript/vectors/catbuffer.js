const sc = require('../src/symbol/models');
const { hexToUint8 } = require('../src/utils/converter');
const YAML = require('yaml');
const yargs = require('yargs');
const fs = require('fs');
const path = require('path');

(() => {
	const areEqual = (lhs, rhs) => {
		if (!!lhs !== !!rhs)
			return false;

		if (lhs.length && rhs.length) {
			if (lhs.length !== rhs.length)
				return false;

			for (let i = 0; i < lhs.length; ++i) {
				if (!areEqual(lhs[i], rhs[i]))
					return false;
			}

			return true;
		}

		if (lhs.bytes && rhs.bytes)
			return areEqual(lhs.bytes, rhs.bytes);

		return lhs === rhs;
	};

	const assertRoundtripBuilder = testCase => {
		// Arrange:
		const payload = hexToUint8(testCase.payload);
		const builder = sc[testCase.schema_name];
		if (!builder)
			throw RangeError(`invalid builder name: ${testCase.schema_name}`);

		// Act:
		const transaction = builder.deserialize(payload);

		// console.log(transaction.toString());

		const serialized = transaction.serialize();

		return areEqual(serialized, payload);
	};

	const runTestSuite = (testSuiteName, filepath) => {
		const fileContent = fs.readFileSync(filepath, 'utf8');
		const testVectors = YAML.parse(fileContent);

		let testCaseNumber = 0;
		let numFailed = 0;
		testVectors.forEach(testCase => {
			testCaseNumber++;
			if (!assertRoundtripBuilder(testCase)) {
				console.log(`${testCase.test_name} failed`);
				numFailed++;
			}
		});

		if (numFailed) {
			console.log(`${testSuiteName} ${numFailed} failures out of ${testCaseNumber}`);
			return false;
		}

		console.log(`${testSuiteName} successes ${testCaseNumber}`);
		return true;
	};

	const args = yargs(process.argv.slice(2))
		.demandOption('vectors', 'path to test-vectors directory')
		.option('blockchain', {
			describe: 'blockchain to run vectors against',
			choices: ['nem', 'symbol'],
			default: 'symbol'
		})
		.argv;

	const dirname = `${args.vectors}/${args.blockchain}/transactions`;
	const testSuiteNames = fs.readdirSync(dirname);
	let numSuitesFailed = 0;
	testSuiteNames.filter(name => -1 === name.indexOf('invalid')).forEach(name => {
		// TODO: skip state tests, discuss on discord
		if (-1 !== name.indexOf('state'))
			return;

		const filepath = path.format({ dir: dirname, base: name });
		if (!runTestSuite(name, filepath))
			++numSuitesFailed;
	});

	if (numSuitesFailed)
		process.exit(1);
})();

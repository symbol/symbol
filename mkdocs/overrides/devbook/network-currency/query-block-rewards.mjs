import { PublicKey } from 'symbol-sdk';
import { Address, Network } from 'symbol-sdk/symbol';

const fmt = (v) => (v / 1e6).toLocaleString(
	'en-US', { minimumFractionDigits: 6 });

const NODE_URL = process.env.NODE_URL ||
	'https://reference.symboltest.net:3001';
console.log(`Using node ${NODE_URL}`);

const BLOCK_HEIGHT = process.env.BLOCK_HEIGHT || '3222290';

// Get the block header
const block = await (await fetch(
	`${NODE_URL}/blocks/${BLOCK_HEIGHT}`)).json();
const signer = Network.TESTNET.publicKeyToAddress(
	new PublicKey(block.block.signerPublicKey));
const beneficiary = block.block.beneficiaryAddress;
console.log(`Block height: ${BLOCK_HEIGHT}`);
console.log(`Signer: ${signer}`);
const beneficiaryB32 = Address.fromDecodedAddressHexString(
	beneficiary);
console.log(`Beneficiary: ${beneficiaryB32}`);

// Get the network sink address
const properties = await (await fetch(
	`${NODE_URL}/network/properties`)).json();
const sinkB32 = properties.chain.harvestNetworkFeeSinkAddress;
const sink = Array.from(new Address(sinkB32).bytes)
	.map(b => b.toString(16).padStart(2, '0')).join('').toUpperCase();
console.log(`Network sink: ${sinkB32}`);

// Get the inflation reward at this height
const inflation = await (await fetch(
	`${NODE_URL}/network/inflation/at/${BLOCK_HEIGHT}`)).json();
const reward = parseInt(inflation.rewardAmount, 10);
console.log(`Inflation reward: ${fmt(reward)} XYM`);

// Get harvest fee receipts for this block
const receipts = await (await fetch(
	`${NODE_URL}/statements/transaction`
	+ `?height=${BLOCK_HEIGHT}&receiptType=8515`)).json();

// Label and display the reward distribution
let total = 0;
console.log('\nReward distribution:');
for (const item of receipts.data) {
	for (const r of item.statement.receipts) {
		if (r.type !== 8515) continue;
		const amount = parseInt(r.amount, 10);
		total += amount;
		let label;
		if (r.targetAddress === sink) {
			label = 'Network sink (5%)';
		} else if (r.targetAddress === beneficiary) {
			label = 'Beneficiary (25%)';
		} else {
			label = 'Harvester';
			const harvester = Address.fromDecodedAddressHexString(
				r.targetAddress);
			console.log(`  ${label}: ${fmt(amount)} XYM`);
			console.log(`  Harvester: ${harvester}`);
			continue;
		}
		console.log(`  ${label}: ${fmt(amount)} XYM`);
	}
}

// Summary
const fees = total - reward;
console.log('\nSummary:');
console.log(`  Total block reward: ${fmt(total)} XYM`);
console.log(`  Inflation: ${fmt(reward)} XYM`);
console.log(`  Transaction fees: ${fmt(fees)} XYM`);

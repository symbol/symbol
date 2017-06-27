import { expect } from 'chai';
import networkInfo from '../../src/model/networkInfo';

describe('network info', () => {
	describe('networks', () => {
		it('defines all known networks', () => {
			// Act:
			const knownNetworks = Object.keys(networkInfo.networks);

			// Assert:
			expect(knownNetworks).to.deep.equal([
				'mijin',
				'mijinTest',
				'public',
				'publicTest'
			]);
		});

		it('defines mijin network', () => {
			// Assert:
			expect(networkInfo.networks.mijin).to.deep.equal({ id: 0x60, bytePrefix: '60', charPrefix: 'M' });
		});

		it('defines mijin test network', () => {
			// Assert:
			expect(networkInfo.networks.mijinTest).to.deep.equal({ id: 0x90, bytePrefix: '90', charPrefix: 'S' });
		});

		it('defines public network', () => {
			// Assert:
			expect(networkInfo.networks.public).to.deep.equal({ id: 0x68, bytePrefix: '68', charPrefix: 'N' });
		});

		it('defines public test network', () => {
			// Assert:
			expect(networkInfo.networks.publicTest).to.deep.equal({ id: 0x98, bytePrefix: '98', charPrefix: 'T' });
		});
	});

	describe('find by id', () => {
		it('can find all known networks', () => {
			// Arrange:
			for (const networkName of Object.keys(networkInfo.networks)) {
				// Act:
				const network = networkInfo.findById(networkInfo.networks[networkName].id);

				// Assert:
				expect(network).to.equal(networkInfo.networks[networkName]);
			}
		});

		it('returns undefined for unknown network', () => {
			// Act:
			const network = networkInfo.findById(0x25);

			// Assert:
			expect(network).to.equal(undefined);
		});
	});

	describe('find by char prefix', () => {
		it('can find all known networks', () => {
			// Arrange:
			for (const networkName of Object.keys(networkInfo.networks)) {
				// Act:
				const network = networkInfo.findByCharPrefix(networkInfo.networks[networkName].charPrefix);

				// Assert:
				expect(network).to.equal(networkInfo.networks[networkName]);
			}
		});

		it('returns undefined for unknown network', () => {
			// Act:
			const network = networkInfo.findByCharPrefix('J');

			// Assert:
			expect(network).to.equal(undefined);
		});
	});
});

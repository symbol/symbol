import { expect } from 'chai';
import networkTime from '../../src/utils/networkTime';

describe('network time', () => {
	it('is positive', () => {
		// Act:
		const time = networkTime.getNetworkTime();

		// Assert:
		expect(time).to.be.at.least(1);
	});

	it('is increasing', () => {
		// Arrange:
		const time1 = networkTime.getNetworkTime();

		// Act:
		setTimeout(() => {
			const time2 = networkTime.getNetworkTime();

			// Assert:
			expect(time1).to.be.below(time2);
		}, 10);
	});
});

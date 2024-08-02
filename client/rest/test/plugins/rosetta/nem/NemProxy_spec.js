/*
 * Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
 * Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
 * All rights reserved.
 *
 * This file is part of Catapult.
 *
 * Catapult is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Catapult is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Catapult.  If not, see <http://www.gnu.org/licenses/>.
 */

import { FetchStubHelper } from './utils/rosettaTestUtils.js';
import NemProxy from '../../../../src/plugins/rosetta/nem/NemProxy.js';
import { RosettaErrorFactory } from '../../../../src/plugins/rosetta/rosettaUtils.js';
import { expect } from 'chai';

describe('NemProxy', () => {
	// region utils

	const TEST_ENDPOINT = 'http://localhost:3456';

	const assertAsyncErrorThrown = async (func, expectedRosettaError) => {
		try {
			await func();
			expect.fail(`no error thrown - expected ${expectedRosettaError.message}`);
		} catch (err) {
			expect(err.apiError).deep.equal(expectedRosettaError.apiError);
		}
	};

	const stubFetchResult = FetchStubHelper.stubPost;
	FetchStubHelper.registerStubCleanup();

	// endregion

	// region fetch

	describe('fetch', () => {
		it('fails when fetch fails (headers)', async () => {
			// Arrange:
			const proxy = new NemProxy(TEST_ENDPOINT);
			stubFetchResult('custom/route', false, { foo: 123, bar: 246 });

			// Act + Assert:
			await assertAsyncErrorThrown(() => proxy.fetch('custom/route'), RosettaErrorFactory.CONNECTION_ERROR);

			expect(global.fetch.callCount).to.equal(1);
		});

		it('fails when fetch fails (body)', async () => {
			// Arrange:
			const proxy = new NemProxy(TEST_ENDPOINT);
			stubFetchResult('custom/route', true, Promise.reject(Error('fetch failed')));

			// Act + Assert:
			await assertAsyncErrorThrown(() => proxy.fetch('custom/route'), RosettaErrorFactory.CONNECTION_ERROR);

			expect(global.fetch.callCount).to.equal(1);
		});

		const assertFetchCalls = expectedResponseOptions => {
			expect(global.fetch.callCount).to.equal(1);
			expect(global.fetch.withArgs(`${TEST_ENDPOINT}/custom/route`, expectedResponseOptions).callCount).to.equal(1);
		};

		it('returns valid response on success (without projection)', async () => {
			// Arrange:
			const proxy = new NemProxy(TEST_ENDPOINT);
			stubFetchResult('custom/route', true, { foo: 123, bar: 246 });

			// Act:
			const result = await proxy.fetch('custom/route');

			// Assert:
			assertFetchCalls({});
			expect(result).to.deep.equal({ foo: 123, bar: 246 });
		});

		it('returns valid response on success (with projection)', async () => {
			// Arrange:
			const proxy = new NemProxy(TEST_ENDPOINT);
			stubFetchResult('custom/route', true, { foo: 123, bar: 246 });

			// Act:
			const result = await proxy.fetch('custom/route', jsonObject => jsonObject.bar);

			// Assert:
			assertFetchCalls({});
			expect(result).to.equal(246);
		});

		it('forwards request options to underlying fetch', async () => {
			// Arrange:
			const proxy = new NemProxy(TEST_ENDPOINT);
			stubFetchResult('custom/route', true, { foo: 123, bar: 246 });

			// Act:
			const result = await proxy.fetch('custom/route', undefined, { method: 'POST' });

			// Assert:
			assertFetchCalls({ method: 'POST' });
			expect(result).to.deep.equal({ foo: 123, bar: 246 });
		});
	});

	// endregion
});

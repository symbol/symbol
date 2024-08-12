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

	// region fetchAll

	describe('fetchAll', () => {
		const TEST_DATA = [
			{ meta: { id: 33 }, payload: { foo: 2, bar: 5 } },
			{ meta: { id: 30 }, payload: { foo: 4, bar: 4 } },
			{ meta: { id: 27 }, payload: { foo: 6, bar: 3 } },
			{ meta: { id: 24 }, payload: { foo: 8, bar: 2 } },
			{ meta: { id: 21 }, payload: { foo: 10, bar: 1 } },
			{ meta: { id: 18 }, payload: { foo: 12, bar: 1 } },
			{ meta: { id: 15 }, payload: { foo: 14, bar: 2 } },
			{ meta: { id: 12 }, payload: { foo: 16, bar: 3 } },
			{ meta: { id: 9 }, payload: { foo: 18, bar: 4 } },
			{ meta: { id: 6 }, payload: { foo: 20, bar: 5 } },
			{ meta: { id: 3 }, payload: { foo: 22, bar: 5 } },
			{ meta: { id: 1 }, payload: { foo: 24, bar: 4 } }
		];

		it('fails when fetch fails (headers)', async () => {
			// Arrange:
			const proxy = new NemProxy(TEST_ENDPOINT);
			stubFetchResult('custom/route?pageSize=5', true, { data: TEST_DATA.slice(0, 5) });
			stubFetchResult('custom/route?id=21&pageSize=5', false, { data: TEST_DATA.slice(5, 10) });
			stubFetchResult('custom/route?id=6&pageSize=5', true, { data: TEST_DATA.slice(10) });

			// Act + Assert:
			await assertAsyncErrorThrown(() => proxy.fetchAll('custom/route', 5), RosettaErrorFactory.CONNECTION_ERROR);

			expect(global.fetch.callCount).to.equal(2);
		});

		it('fails when fetch fails (body)', async () => {
			// Arrange:
			const proxy = new NemProxy(TEST_ENDPOINT);
			stubFetchResult('custom/route?pageSize=5', true, { data: TEST_DATA.slice(0, 5) });
			stubFetchResult('custom/route?id=21&pageSize=5', true, Promise.reject(Error('fetch failed')));
			stubFetchResult('custom/route?id=6&pageSize=5', true, { data: TEST_DATA.slice(10) });

			// Act + Assert:
			await assertAsyncErrorThrown(() => proxy.fetchAll('custom/route', 5), RosettaErrorFactory.CONNECTION_ERROR);

			expect(global.fetch.callCount).to.equal(2);
		});

		const addBasicFetchAllSuccessTests = (urlPath, expectedUrlPathPrefix) => {
			const stubSuccessFetchResults = () => {
				stubFetchResult(`${expectedUrlPathPrefix}pageSize=5`, true, { data: TEST_DATA.slice(0, 5) });
				stubFetchResult(`${expectedUrlPathPrefix}id=21&pageSize=5`, true, { data: TEST_DATA.slice(5, 10) });
				stubFetchResult(`${expectedUrlPathPrefix}id=6&pageSize=5`, true, { data: TEST_DATA.slice(10) });
			};

			const assertFetchCalls = expectedResponseOptions => {
				const makeUrlForId = id => (
					undefined === id
						? `${TEST_ENDPOINT}/${expectedUrlPathPrefix}pageSize=5`
						: `${TEST_ENDPOINT}/${expectedUrlPathPrefix}id=${id}&pageSize=5`
				);
				expect(global.fetch.callCount).to.equal(3);
				expect(global.fetch.withArgs(makeUrlForId(undefined), expectedResponseOptions).callCount).to.equal(1);
				expect(global.fetch.withArgs(makeUrlForId(21), expectedResponseOptions).callCount).to.equal(1);
				expect(global.fetch.withArgs(makeUrlForId(6), expectedResponseOptions).callCount).to.equal(1);
			};

			it('returns valid response on success (without projection)', async () => {
				// Arrange:
				const proxy = new NemProxy(TEST_ENDPOINT);
				stubSuccessFetchResults();

				// Act:
				const result = await proxy.fetchAll(urlPath, 5);

				// Assert:
				assertFetchCalls({});
				expect(result).to.deep.equal(TEST_DATA);
			});

			it('returns valid response on success (with projection)', async () => {
				// Arrange:
				const proxy = new NemProxy(TEST_ENDPOINT);
				stubSuccessFetchResults();

				// Act:
				const result = await proxy.fetchAll(urlPath, 5, jsonObject => jsonObject.payload.bar);

				// Assert:
				assertFetchCalls({});
				expect(result).to.deep.equal(TEST_DATA.map(obj => obj.payload.bar));
			});

			it('forwards request options to underlying fetch', async () => {
				// Arrange:
				const proxy = new NemProxy(TEST_ENDPOINT);
				stubSuccessFetchResults();

				// Act:
				const result = await proxy.fetchAll(urlPath, 5, undefined, { method: 'POST' });

				// Assert:
				assertFetchCalls({ method: 'POST' });
				expect(result).to.deep.equal(TEST_DATA);
			});
		};

		describe('with no query params', () => {
			addBasicFetchAllSuccessTests('custom/route', 'custom/route?');
		});

		describe('with query params', () => {
			addBasicFetchAllSuccessTests('custom/route?q=alpha', 'custom/route?q=alpha&');
		});
	});

	// endregion

	// region mosaicProperties

	describe('mosaicProperties', () => {
		const assertCanRetriveProperties = async options => {
			// Arrange:
			const proxy = new NemProxy(TEST_ENDPOINT);
			stubFetchResult('namespace/definition/page?namespace=foo.bar&pageSize=100', true, {
				data: [
					{ mosaic: { id: { namespaceId: 'foo.bar', name: 'tokens' } } },
					{
						mosaic: {
							id: { namespaceId: 'foo.bar', name: 'coins' },
							properties: options.properties,
							levy: options.levy
						}
					},
					{ mosaic: { id: { namespaceId: 'foo.bar', name: 'chips' } } }
				]
			});

			// Act:
			const mosaicProperties = await proxy.mosaicProperties({ namespaceId: 'foo.bar', name: 'coins' });

			// Assert:
			expect(mosaicProperties).to.deep.equal(options.expectedProperties);
		};

		it('can retrieve properties for mosaic with default divisibility', () => assertCanRetriveProperties({
			properties: [
				{ name: 'initialSupply', value: '123000' },
				{ name: 'supplyMutable', value: 'false' }
			],
			levy: {},
			expectedProperties: {
				divisibility: 0,
				levy: undefined
			}
		}));

		it('can retrieve properties for mosaic with custom divisibility', () => assertCanRetriveProperties({
			properties: [
				{ name: 'initialSupply', value: '123000' },
				{ name: 'divisibility', value: '4' },
				{ name: 'supplyMutable', value: 'false' }
			],
			levy: {},
			expectedProperties: {
				divisibility: 4,
				levy: undefined
			}
		}));

		it('can retrieve properties for mosaic with levy (absolute)', () => assertCanRetriveProperties({
			properties: [
				{ name: 'initialSupply', value: '123000' },
				{ name: 'divisibility', value: '4' },
				{ name: 'supplyMutable', value: 'false' }
			],
			levy: {
				type: 1,
				recipient: 'TD3RXTHBLK6J3UD2BH2PXSOFLPWZOTR34WCG4HXH',
				mosaicId: { namespaceId: 'nem', name: 'xem' },
				fee: 10
			},
			expectedProperties: {
				divisibility: 4,
				levy: {
					mosaicId: { namespaceId: 'nem', name: 'xem' },
					recipientAddress: 'TD3RXTHBLK6J3UD2BH2PXSOFLPWZOTR34WCG4HXH',
					isAbsolute: true,
					fee: 10
				}
			}
		}));

		it('can retrieve properties for mosaic with levy (relative)', () => assertCanRetriveProperties({
			properties: [
				{ name: 'initialSupply', value: '123000' },
				{ name: 'divisibility', value: '4' },
				{ name: 'supplyMutable', value: 'false' }
			],
			levy: {
				type: 2,
				recipient: 'TD3RXTHBLK6J3UD2BH2PXSOFLPWZOTR34WCG4HXH',
				mosaicId: { namespaceId: 'nem', name: 'xem' },
				fee: 10
			},
			expectedProperties: {
				divisibility: 4,
				levy: {
					mosaicId: { namespaceId: 'nem', name: 'xem' },
					recipientAddress: 'TD3RXTHBLK6J3UD2BH2PXSOFLPWZOTR34WCG4HXH',
					isAbsolute: false,
					fee: 10
				}
			}
		}));

		it('can retrieve properties for mosaic (cached)', async () => {
			// Arrange:
			const proxy = new NemProxy(TEST_ENDPOINT);
			stubFetchResult('namespace/definition/page?namespace=foo.bar&pageSize=100', true, {
				data: [
					{
						mosaic: {
							id: { namespaceId: 'foo.bar', name: 'coins' },
							properties: [
								{ name: 'initialSupply', value: '123000' },
								{ name: 'divisibility', value: '4' },
								{ name: 'supplyMutable', value: 'false' }
							],
							levy: {
								type: 2,
								recipient: 'TD3RXTHBLK6J3UD2BH2PXSOFLPWZOTR34WCG4HXH',
								mosaicId: { namespaceId: 'nem', name: 'xem' },
								fee: 10
							}
						}
					}
				]
			});

			// Act:
			await proxy.mosaicProperties({ namespaceId: 'foo.bar', name: 'coins' });
			await proxy.mosaicProperties({ namespaceId: 'foo.bar', name: 'coins' });
			const mosaicProperties = await proxy.mosaicProperties({ namespaceId: 'foo.bar', name: 'coins' });

			// Assert: only initial call was made
			expect(global.fetch.callCount).to.equal(1);
			expect(mosaicProperties).to.deep.equal({
				divisibility: 4,
				levy: {
					mosaicId: { namespaceId: 'nem', name: 'xem' },
					recipientAddress: 'TD3RXTHBLK6J3UD2BH2PXSOFLPWZOTR34WCG4HXH',
					isAbsolute: false,
					fee: 10
				}
			});
		});

		it('fails when no matching mosaic is found', async () => {
			// Arrange:
			const proxy = new NemProxy(TEST_ENDPOINT);
			stubFetchResult('namespace/definition/page?namespace=foo.bar&pageSize=100', true, {
				data: [
					{ mosaic: { id: { namespaceId: 'foo.bar', name: 'tokens' } } },
					{ mosaic: { id: { namespaceId: 'foo.bar', name: 'coupons' } } },
					{ mosaic: { id: { namespaceId: 'foo.bar', name: 'chips' } } }
				]
			});

			// Act + Assert:
			await assertAsyncErrorThrown(
				() => proxy.mosaicProperties({ namespaceId: 'foo.bar', name: 'coins' }),
				RosettaErrorFactory.INTERNAL_SERVER_ERROR
			);
		});

		it('fails when fetch fails (namespace/definition/page)', async () => {
			// Arrange:
			const proxy = new NemProxy(TEST_ENDPOINT);
			stubFetchResult('namespace/definition/page?namespace=foo.bar&pageSize=100', false, {
				data: [
					{
						mosaic: {
							id: { namespaceId: 'foo.bar', name: 'coins' },
							properties: [],
							levy: {}
						}
					}
				]
			});

			// Act + Assert:
			await assertAsyncErrorThrown(
				() => proxy.mosaicProperties({ namespaceId: 'foo.bar', name: 'coins' }),
				RosettaErrorFactory.CONNECTION_ERROR
			);
		});
	});

	// endregion
});

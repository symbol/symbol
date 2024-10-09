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
import { RosettaErrorFactory } from '../../../../src/plugins/rosetta/rosettaUtils.js';
import CatapultProxy from '../../../../src/plugins/rosetta/symbol/CatapultProxy.js';
import { expect } from 'chai';

describe('CatapultProxy', () => {
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
			const proxy = new CatapultProxy(TEST_ENDPOINT);
			stubFetchResult('custom/route', false, { foo: 123, bar: 246 });

			// Act + Assert:
			await assertAsyncErrorThrown(() => proxy.fetch('custom/route'), RosettaErrorFactory.CONNECTION_ERROR);

			expect(global.fetch.callCount).to.equal(1);
		});

		it('fails when fetch fails (body)', async () => {
			// Arrange:
			const proxy = new CatapultProxy(TEST_ENDPOINT);
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
			const proxy = new CatapultProxy(TEST_ENDPOINT);
			stubFetchResult('custom/route', true, { foo: 123, bar: 246 });

			// Act:
			const result = await proxy.fetch('custom/route');

			// Assert:
			assertFetchCalls({});
			expect(result).to.deep.equal({ foo: 123, bar: 246 });
		});

		it('returns valid response on success (with projection)', async () => {
			// Arrange:
			const proxy = new CatapultProxy(TEST_ENDPOINT);
			stubFetchResult('custom/route', true, { foo: 123, bar: 246 });

			// Act:
			const result = await proxy.fetch('custom/route', jsonObject => jsonObject.bar);

			// Assert:
			assertFetchCalls({});
			expect(result).to.equal(246);
		});

		it('forwards request options to underlying fetch', async () => {
			// Arrange:
			const proxy = new CatapultProxy(TEST_ENDPOINT);
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
			{ foo: 2, bar: 5 },
			{ foo: 4, bar: 4 },
			{ foo: 6, bar: 3 },
			{ foo: 8, bar: 2 },
			{ foo: 10, bar: 1 },
			{ foo: 12, bar: 1 },
			{ foo: 14, bar: 2 },
			{ foo: 16, bar: 3 },
			{ foo: 18, bar: 4 },
			{ foo: 20, bar: 5 },
			{ foo: 22, bar: 5 },
			{ foo: 24, bar: 4 }
		];

		it('fails when fetch fails (headers)', async () => {
			// Arrange:
			const proxy = new CatapultProxy(TEST_ENDPOINT);
			stubFetchResult('custom/route?pageNumber=1&pageSize=5', true, { data: TEST_DATA.slice(0, 5) });
			stubFetchResult('custom/route?pageNumber=2&pageSize=5', false, { data: TEST_DATA.slice(5, 10) });
			stubFetchResult('custom/route?pageNumber=3&pageSize=5', true, { data: TEST_DATA.slice(10) });

			// Act + Assert:
			await assertAsyncErrorThrown(() => proxy.fetchAll('custom/route', 5), RosettaErrorFactory.CONNECTION_ERROR);

			expect(global.fetch.callCount).to.equal(2);
		});

		it('fails when fetch fails (body)', async () => {
			// Arrange:
			const proxy = new CatapultProxy(TEST_ENDPOINT);
			stubFetchResult('custom/route?pageNumber=1&pageSize=5', true, { data: TEST_DATA.slice(0, 5) });
			stubFetchResult('custom/route?pageNumber=2&pageSize=5', true, Promise.reject(Error('fetch failed')));
			stubFetchResult('custom/route?pageNumber=3&pageSize=5', true, { data: TEST_DATA.slice(10) });

			// Act + Assert:
			await assertAsyncErrorThrown(() => proxy.fetchAll('custom/route', 5), RosettaErrorFactory.CONNECTION_ERROR);

			expect(global.fetch.callCount).to.equal(2);
		});

		const addBasicFetchAllSuccessTests = (urlPath, expectedUrlPathPrefix) => {
			const stubSuccessFetchResults = () => {
				stubFetchResult(`${expectedUrlPathPrefix}pageNumber=1&pageSize=5`, true, { data: TEST_DATA.slice(0, 5) });
				stubFetchResult(`${expectedUrlPathPrefix}pageNumber=2&pageSize=5`, true, { data: TEST_DATA.slice(5, 10) });
				stubFetchResult(`${expectedUrlPathPrefix}pageNumber=3&pageSize=5`, true, { data: TEST_DATA.slice(10) });
			};

			const assertFetchCalls = expectedResponseOptions => {
				const makeUrlForPage = pageNumber => `${TEST_ENDPOINT}/${expectedUrlPathPrefix}pageNumber=${pageNumber}&pageSize=5`;
				expect(global.fetch.callCount).to.equal(3);
				expect(global.fetch.withArgs(makeUrlForPage(1), expectedResponseOptions).callCount).to.equal(1);
				expect(global.fetch.withArgs(makeUrlForPage(2), expectedResponseOptions).callCount).to.equal(1);
				expect(global.fetch.withArgs(makeUrlForPage(3), expectedResponseOptions).callCount).to.equal(1);
			};

			it('returns valid response on success (without projection)', async () => {
				// Arrange:
				const proxy = new CatapultProxy(TEST_ENDPOINT);
				stubSuccessFetchResults();

				// Act:
				const result = await proxy.fetchAll(urlPath, 5);

				// Assert:
				assertFetchCalls({});
				expect(result).to.deep.equal(TEST_DATA);
			});

			it('returns valid response on success (with projection)', async () => {
				// Arrange:
				const proxy = new CatapultProxy(TEST_ENDPOINT);
				stubSuccessFetchResults();

				// Act:
				const result = await proxy.fetchAll(urlPath, 5, jsonObject => jsonObject.bar);

				// Assert:
				assertFetchCalls({});
				expect(result).to.deep.equal(TEST_DATA.map(obj => obj.bar));
			});

			it('forwards request options to underlying fetch', async () => {
				// Arrange:
				const proxy = new CatapultProxy(TEST_ENDPOINT);
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

	// region cache

	describe('cache', () => {
		const setCacheFetchResults = (failureUrlPath = undefined) => {
			stubFetchResult('node/info', 'node/info' !== failureUrlPath, { node: 'alpha' });
			stubFetchResult('network/properties', 'network/properties' !== failureUrlPath, { network: 'beta' });
			stubFetchResult('blocks/1', 'blocks/1' !== failureUrlPath, { height: 'gamma' });
		};

		const runGlobalCacheQueryTest = async (action, expectedResult) => {
			// Arrange:
			const proxy = new CatapultProxy(TEST_ENDPOINT);
			setCacheFetchResults();

			// Act:
			const result = await action(proxy);

			// Assert: only initial calls were made
			expect(global.fetch.callCount).to.equal(3);
			expect(result).to.deep.equal(expectedResult);
		};

		const runGlobalCacheErrorRetryTest = async (failureUrlPath, action, expectedResult) => {
			// Arrange:
			const proxy = new CatapultProxy(TEST_ENDPOINT);
			setCacheFetchResults(failureUrlPath);

			// Sanity:
			await assertAsyncErrorThrown(() => action(proxy), RosettaErrorFactory.CONNECTION_ERROR);

			// Arrange:
			setCacheFetchResults();

			// Act:
			const result = await action(proxy);

			// Assert: first (failure) and second (success) calls were made
			expect(global.fetch.callCount).to.equal(6);
			expect(result).to.deep.equal(expectedResult);
		};

		const addCachePropertyTests = (propertyName, urlPath, expectedResult, additionalTests = undefined) => {
			describe(propertyName, () => {
				it('can retrieve', () => runGlobalCacheQueryTest(proxy => proxy[propertyName](), expectedResult));

				it('can retrieve (cached)', () => runGlobalCacheQueryTest(async proxy => {
					await proxy[propertyName]();
					await proxy[propertyName]();
					return proxy[propertyName]();
				}, expectedResult));

				it('can retrieve after failure', () => runGlobalCacheErrorRetryTest(
					urlPath,
					proxy => proxy[propertyName](),
					expectedResult
				));

				if (additionalTests)
					additionalTests();
			});
		};

		addCachePropertyTests('nodeInfo', 'node/info', { node: 'alpha' });
		addCachePropertyTests('networkProperties', 'network/properties', { network: 'beta' }, () => {
			it('post processes currencyMosaicId', async () => {
				// Arrange:
				const proxy = new CatapultProxy(TEST_ENDPOINT);
				setCacheFetchResults();
				stubFetchResult('network/properties', true, {
					tag: 'beta',
					chain: {
						currencyMosaicId: '0x664E\'D36A\'3138\'66F5'
					}
				});

				// Act:
				const result = await proxy.networkProperties();

				// Assert: only initial calls were made
				expect(global.fetch.callCount).to.equal(3);
				expect(result).to.deep.equal({
					tag: 'beta',
					chain: {
						currencyMosaicId: '0x664ED36A313866F5'
					}
				});
			});

			describe('post processes epochAdjustment', () => {
				const assertPostProcessesEpochAdjustment = async (unparsedValue, parsedValue) => {
					// Arrange:
					const proxy = new CatapultProxy(TEST_ENDPOINT);
					setCacheFetchResults();
					stubFetchResult('network/properties', true, {
						tag: 'beta',
						network: {
							epochAdjustment: unparsedValue
						}
					});

					// Act:
					const result = await proxy.networkProperties();

					// Assert: only initial calls were made
					expect(global.fetch.callCount).to.equal(3);
					expect(result).to.deep.equal({
						tag: 'beta',
						network: {
							epochAdjustment: parsedValue
						}
					});
				};

				it('as hours', () => assertPostProcessesEpochAdjustment('1122h', 1122n * 60n * 60n * 1000n));
				it('as minutes', () => assertPostProcessesEpochAdjustment('1122m', 1122n * 60n * 1000n));
				it('as seconds', () => assertPostProcessesEpochAdjustment('1122s', 1122n * 1000n));
				it('as milliseconds', () => assertPostProcessesEpochAdjustment('1122ms', 1122n));

				it('as other fails', async () => {
					// Arrange:
					const proxy = new CatapultProxy(TEST_ENDPOINT);
					setCacheFetchResults();
					stubFetchResult('network/properties', true, {
						tag: 'beta',
						network: {
							epochAdjustment: '1122d'
						}
					});

					// Act + Assert:
					await assertAsyncErrorThrown(() => proxy.networkProperties(), RosettaErrorFactory.INTERNAL_SERVER_ERROR);
				});
			});
		});
		addCachePropertyTests('nemesisBlock', 'blocks/1', { height: 'gamma' });

		it('handles race condition when multiple cache fills are triggered in parallel', async () => {
			// Arrange:
			const proxy = new CatapultProxy(TEST_ENDPOINT);
			stubFetchResult('node/info', true, { tag: 'alpha' });
			stubFetchResult('network/properties', true, { tag: 'beta', network: { epochAdjustment: '1122s' } });
			stubFetchResult('blocks/1', true, { tag: 'zeta' });

			// Act:
			const results = await Promise.all([
				proxy.nodeInfo(),
				proxy.networkProperties(),
				proxy.nemesisBlock()
			]);

			// Assert: multiple calls were made
			expect(global.fetch.callCount).to.equal(9);
			expect(results).to.deep.equal([
				{ tag: 'alpha' },
				{ tag: 'beta', network: { epochAdjustment: 1122n * 1000n } },
				{ tag: 'zeta' }
			]);
		});
	});

	// endregion

	// region resolveMosaicId / resolveAddress

	const addResolveResolvableTests = options => {
		describe('resolved', () => {
			it('can resolve without network access', async () => {
				// Arrange:
				const proxy = new CatapultProxy(TEST_ENDPOINT);

				// Act:
				const resolvedValue = await proxy[options.resolveFunctionName](options.resolvedValue);

				// Assert:
				expect(resolvedValue).to.equal(options.resolvedValue);
			});
		});

		describe('unresolved (current)', () => {
			it('can resolve value', async () => {
				// Arrange:
				const proxy = new CatapultProxy(TEST_ENDPOINT);
				stubFetchResult('namespaces/9234567890ABCDEF', true, { namespace: { alias: options.aliasObject } });

				// Act:
				const resolvedValue = await proxy[options.resolveFunctionName](options.unresolvedValue);

				// Assert:
				expect(resolvedValue).to.equal(options.resolvedValue);
			});

			it('fails when fetch fails (namespaces)', async () => {
				// Arrange:
				const proxy = new CatapultProxy(TEST_ENDPOINT);
				stubFetchResult('namespaces/9234567890ABCDEF', false, { namespace: { alias: options.aliasObject } });

				// Act + Assert:
				await assertAsyncErrorThrown(
					() => proxy[options.resolveFunctionName](options.unresolvedValue),
					RosettaErrorFactory.CONNECTION_ERROR
				);
			});
		});

		describe('unresolved (historical)', () => {
			const makeResolutionStatement = (unresolved, resolutionEntries) => ({ statement: { unresolved, resolutionEntries } });
			const makeResolutionEntry = (resolved, primaryId, secondaryId) => ({ resolved, source: { primaryId, secondaryId } });

			it('cannot resolve value when no matching statements exist', async () => {
				// Arrange:
				const proxy = new CatapultProxy(TEST_ENDPOINT);
				stubFetchResult(`statements/resolutions/${options.resolutionType}?height=1234`, true, {
					data: [
						makeResolutionStatement('AAAAAAAAAAAAAAAA', [
							makeResolutionEntry('BBBBBBBBBBBBBBBB', 1, 0)
						])
					]
				});

				// Act + Assert:
				await assertAsyncErrorThrown(
					() => proxy[options.resolveFunctionName](options.unresolvedValue, { height: 1234n, primaryId: 2, secondaryId: 3 }),
					RosettaErrorFactory.INTERNAL_SERVER_ERROR
				);
			});

			it('cannot resolve value when no matching resolution entries exist', async () => {
				// Arrange:
				const proxy = new CatapultProxy(TEST_ENDPOINT);
				stubFetchResult(`statements/resolutions/${options.resolutionType}?height=1234`, true, {
					data: [
						makeResolutionStatement(options.unresolvedValueString, [
							makeResolutionEntry(options.resolvedValueString, 2, 4),
							makeResolutionEntry(options.otherResolvedValueString2, 3, 0)
						])
					]
				});

				// Act + Assert:
				await assertAsyncErrorThrown(
					() => proxy[options.resolveFunctionName](options.unresolvedValue, { height: 1234n, primaryId: 2, secondaryId: 3 }),
					RosettaErrorFactory.INTERNAL_SERVER_ERROR
				);
			});

			const assertCanResolveWhenMatchingResolutionEntriesExist = async data => {
				// Arrange:
				const proxy = new CatapultProxy(TEST_ENDPOINT);
				stubFetchResult(`statements/resolutions/${options.resolutionType}?height=1234`, true, { data });

				// Act:
				const resolvedValue = await proxy[options.resolveFunctionName](options.unresolvedValue, {
					height: 1234n,
					primaryId: 2,
					secondaryId: 3
				});

				// Assert:
				expect(resolvedValue).to.equal(options.resolvedValue);
			};

			it('can resolve value when matching resolution entries exist (primary GT secondary LT)', async () =>
				assertCanResolveWhenMatchingResolutionEntriesExist([
					makeResolutionStatement(options.unresolvedValueString, [
						makeResolutionEntry(options.otherResolvedValueString1, 1, 4),
						makeResolutionEntry(options.resolvedValueString, 1, 5),
						makeResolutionEntry(options.otherResolvedValueString2, 4, 0)
					])
				]));

			it('can resolve value when matching resolution entries exist (primary EQ secondary GT)', async () =>
				assertCanResolveWhenMatchingResolutionEntriesExist([
					makeResolutionStatement(options.unresolvedValueString, [
						makeResolutionEntry(options.otherResolvedValueString1, 2, 1),
						makeResolutionEntry(options.resolvedValueString, 2, 2),
						makeResolutionEntry(options.otherResolvedValueString2, 2, 6)
					])
				]));

			it('can resolve value when matching resolution entries exist (primary EQ secondary EQ)', async () =>
				assertCanResolveWhenMatchingResolutionEntriesExist([
					makeResolutionStatement(options.unresolvedValueString, [
						makeResolutionEntry(options.otherResolvedValueString1, 2, 2),
						makeResolutionEntry(options.resolvedValueString, 2, 3),
						makeResolutionEntry(options.otherResolvedValueString2, 2, 4)
					])
				]));

			it('fails when fetch fails (statements)', async () => {
				// Arrange:
				const proxy = new CatapultProxy(TEST_ENDPOINT);
				stubFetchResult(`statements/resolutions/${options.resolutionType}?height=1234`, false, {
					data: [
						makeResolutionStatement(options.unresolvedValueString, [
							makeResolutionEntry(options.otherResolvedValueString1, 1, 3),
							makeResolutionEntry(options.resolvedValueString, 2, 2),
							makeResolutionEntry(options.otherResolvedValueString2, 3, 0)
						])
					]
				});

				// Act + Assert:
				await assertAsyncErrorThrown(
					() => proxy[options.resolveFunctionName](options.unresolvedValue, { height: 1234n, primaryId: 2, secondaryId: 3 }),
					RosettaErrorFactory.CONNECTION_ERROR
				);
			});
		});
	};

	describe('resolveMosaicId', () => {
		addResolveResolvableTests({
			resolveFunctionName: 'resolveMosaicId',
			resolutionType: 'mosaic',

			unresolvedValue: 0x9234567890ABCDEFn,
			resolvedValue: 0x0034567890ABCDEFn,
			aliasObject: { mosaicId: '0034567890ABCDEF' },

			unresolvedValueString: '9234567890ABCDEF',
			resolvedValueString: '0034567890ABCDEF',
			otherResolvedValueString1: '0234567890ABCDEF',
			otherResolvedValueString2: '2234567890ABCDEF'
		});

		it('can cache permanent alias', async () => {
			// Arrange:
			const proxy = new CatapultProxy(TEST_ENDPOINT);
			stubFetchResult('namespaces/9234567890ABCDEF', true, {
				namespace: {
					alias: { mosaicId: '0034567890ABCDEF' },
					endHeight: '18446744073709551615'
				}
			});

			// Act:
			const resolvedValue1 = await proxy.resolveMosaicId(0x9234567890ABCDEFn);
			const resolvedValue2 = await proxy.resolveMosaicId(0x9234567890ABCDEFn);

			// Assert: only one call was made (first result was cached)
			expect(global.fetch.callCount).to.equal(1);

			expect(resolvedValue1).to.equal(0x0034567890ABCDEFn);
			expect(resolvedValue2).to.equal(0x0034567890ABCDEFn);
		});

		it('cannot cache temporary alias', async () => {
			// Arrange:
			const proxy = new CatapultProxy(TEST_ENDPOINT);
			stubFetchResult('namespaces/9234567890ABCDEF', true, {
				namespace: {
					alias: { mosaicId: '0034567890ABCDEF' },
					endHeight: '1000'
				}
			});

			// Act:
			const resolvedValue1 = await proxy.resolveMosaicId(0x9234567890ABCDEFn);
			const resolvedValue2 = await proxy.resolveMosaicId(0x9234567890ABCDEFn);

			// Assert: call was made each time (result was not cached)
			expect(global.fetch.callCount).to.equal(2);

			expect(resolvedValue1).to.equal(0x0034567890ABCDEFn);
			expect(resolvedValue2).to.equal(0x0034567890ABCDEFn);
		});
	});

	describe('resolveAddress', () => {
		addResolveResolvableTests({
			resolveFunctionName: 'resolveAddress',
			resolutionType: 'address',

			unresolvedValue: '999234567890ABCDEF000000000000000000000000000000',
			resolvedValue: '98A21C834575E3D58DD7AFB5881DD8D52BA6F787853E6CC0',
			aliasObject: { address: '98A21C834575E3D58DD7AFB5881DD8D52BA6F787853E6CC0' },

			unresolvedValueString: '999234567890ABCDEF000000000000000000000000000000',
			resolvedValueString: '98A21C834575E3D58DD7AFB5881DD8D52BA6F787853E6CC0',
			otherResolvedValueString1: '981CDCF02042C8B916C913AADBCD78142D2C37DC4075C09E',
			otherResolvedValueString2: '98C52CCDC2E593413211E81DC90411660BDD33D64C01432D'
		});
	});

	// endregion

	// region mosaicProperties

	describe('mosaicProperties', () => {
		it('can retrieve properties for mosaic with name', async () => {
			// Arrange:
			const proxy = new CatapultProxy(TEST_ENDPOINT);
			stubFetchResult('mosaics/0034567890ABCDEF', true, { mosaic: { divisibility: 3 } });
			stubFetchResult('namespaces/mosaic/names', true, { mosaicNames: [{ names: ['alpha', 'beta'] }] });

			// Act:
			const mosaicProperties = await proxy.mosaicProperties(0x0034567890ABCDEFn);

			// Assert:
			expect(mosaicProperties).to.deep.equal({
				id: '0034567890ABCDEF',
				name: 'alpha',
				divisibility: 3
			});
		});

		it('can retrieve properties for mosaic without name', async () => {
			// Arrange:
			const proxy = new CatapultProxy(TEST_ENDPOINT);
			stubFetchResult('mosaics/0034567890ABCDEF', true, { mosaic: { divisibility: 3 } });
			stubFetchResult('namespaces/mosaic/names', true, { mosaicNames: [{ names: [] }] });

			// Act:
			const mosaicProperties = await proxy.mosaicProperties(0x0034567890ABCDEFn);

			// Assert:
			expect(mosaicProperties).to.deep.equal({
				id: '0034567890ABCDEF',
				name: '0034567890ABCDEF',
				divisibility: 3
			});
		});

		it('can retrieve properties for mosaic with name (cached)', async () => {
			// Arrange:
			const proxy = new CatapultProxy(TEST_ENDPOINT);
			stubFetchResult('mosaics/0034567890ABCDEF', true, { mosaic: { divisibility: 3 } });
			stubFetchResult('namespaces/mosaic/names', true, { mosaicNames: [{ names: ['alpha', 'beta'] }] });

			// Act:
			await proxy.mosaicProperties(0x0034567890ABCDEFn);
			await proxy.mosaicProperties(0x0034567890ABCDEFn);
			const mosaicProperties = await proxy.mosaicProperties(0x0034567890ABCDEFn);

			// Assert: only initial calls were made
			expect(global.fetch.callCount).to.equal(2);
			expect(mosaicProperties).to.deep.equal({
				id: '0034567890ABCDEF',
				name: 'alpha',
				divisibility: 3
			});
		});

		it('fails when fetch fails (mosaics)', async () => {
			// Arrange:
			const proxy = new CatapultProxy(TEST_ENDPOINT);
			stubFetchResult('mosaics/0034567890ABCDEF', false, { mosaic: { divisibility: 3 } });
			stubFetchResult('namespaces/mosaic/names', true, { mosaicNames: [{ names: ['alpha', 'beta'] }] });

			// Act + Assert:
			await assertAsyncErrorThrown(() => proxy.mosaicProperties(0x0034567890ABCDEFn), RosettaErrorFactory.CONNECTION_ERROR);
		});

		it('fails when fetch fails (namespaces/mosaic/names)', async () => {
			// Arrange:
			const proxy = new CatapultProxy(TEST_ENDPOINT);
			stubFetchResult('mosaics/0034567890ABCDEF', true, { mosaic: { divisibility: 3 } });
			stubFetchResult('namespaces/mosaic/names', false, { mosaicNames: [{ names: ['alpha', 'beta'] }] });

			// Act + Assert:
			await assertAsyncErrorThrown(() => proxy.mosaicProperties(0x0034567890ABCDEFn), RosettaErrorFactory.CONNECTION_ERROR);
		});
	});

	// endregion
});

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

const formatters = require('../../src/server/formatters');
const { expect } = require('chai');

describe('formatters', () => {
	const createFormatters = name => formatters.create({
		[name]: {
			chainStatistic: {
				format: chainStatistic => {
					const formatUint64 = uint64 => (uint64 ? [uint64[0], uint64[1] * 2] : undefined);
					const formatChainStatisticCurrent = chainStatisticCurrent => ({
						height: formatUint64(chainStatisticCurrent.height),
						scoreLow: formatUint64(chainStatisticCurrent.scoreLow),
						scoreHigh: formatUint64(chainStatisticCurrent.scoreHigh)
					});

					return {
						id: chainStatistic.id,
						current: formatChainStatisticCurrent(chainStatistic.current)
					};
				}
			}
		}
	});

	const addBasicObjectFormattingTests = assertJsonFormat => {
		// region non-error

		it('can format null object', () => {
			// Arrange:
			const object = null;

			// Assert:
			assertJsonFormat(object, 'null', undefined);
		});

		it('can format basic object', () => {
			// Arrange:
			const object = { foo: 1, bar: 7 };

			// Assert:
			assertJsonFormat(object, '{"foo":1,"bar":7}', undefined);
		});

		it('can format catapult object', () => {
			// Arrange:
			const object = {
				payload: {
					current: {
						height: [1, 2],
						scoreLow: [112233, 8899],
						scoreHigh: [4, 3]
					}
				},
				type: 'chainStatistic'
			};

			// Assert: formatter doubles high part
			assertJsonFormat(object, '{"current":{"height":[1,4],"scoreLow":[112233,17798],"scoreHigh":[4,6]}}', undefined);
		});

		it('can format catapult object with page structure', () => {
			// Arrange:
			const object = {
				payload: {
					data: [{
						current: {
							height: [1, 2],
							scoreLow: [112233, 8899],
							scoreHigh: [4, 3]
						}
					}],
					pagination: {}
				},
				type: 'chainStatistic',
				structure: 'page'
			};

			// Assert: formatter doubles high part
			assertJsonFormat(
				object, '{"data":[{"current":{"height":[1,4],"scoreLow":[112233,17798],"scoreHigh":[4,6]}}],"pagination":{}}', undefined
			);
		});

		it('can format catapult object array', () => {
			// Arrange:
			const object = {
				payload: [
					{ current: { height: [1, 2] } },
					{ current: { height: [8, 7] } }
				],
				type: 'chainStatistic'
			};

			// Assert: formatter doubles high part
			assertJsonFormat(object, '[{"current":{"height":[1,4]}},{"current":{"height":[8,14]}}]', undefined);
		});

		// endregion

		// region error

		it('can format empty error object', () => {
			// Arrange:
			const object = new Error();

			// Assert:
			assertJsonFormat(object, '{"message":""}', 500);
		});

		it('can format error object with message', () => {
			// Arrange:
			const object = new Error();
			object.message = 'bad message';

			// Assert:
			assertJsonFormat(object, '{"message":"bad message"}', 500);
		});

		it('can format error object with message and status code', () => {
			// Arrange:
			const object = new Error();
			object.message = 'bad message';
			object.statusCode = 404;

			// Assert:
			assertJsonFormat(object, '{"message":"bad message"}', 404);
		});

		it('can format error object with body', () => {
			// Arrange: note that body takes precedence
			const object = new Error();
			object.body = { foo: 1, bar: 7 };
			object.message = 'bad message';

			// Assert:
			assertJsonFormat(object, '{"foo":1,"bar":7}', 500);
		});

		it('can format error object with body and status code', () => {
			// Arrange: note that body takes precedence
			const object = new Error();
			object.body = { foo: 1, bar: 7 };
			object.message = 'bad message';
			object.statusCode = 404;

			// Assert:
			assertJsonFormat(object, '{"foo":1,"bar":7}', 404);
		});

		// endregion
	};

	describe('json', () => {
		addBasicObjectFormattingTests((object, expectedJson, expectedStatusCode) => {
			// Arrange:
			const req = {};

			const resHeaders = [];
			const res = {
				setHeader: (key, value) => {
					resHeaders.push({ key, value });
				}
			};

			// Act:
			const result = createFormatters('json').json(req, res, object);

			// Assert:
			expect(res.statusCode).to.equal(expectedStatusCode);

			expect(resHeaders.length).to.equal(1);
			expect(resHeaders[0]).to.deep.equal({
				key: 'Content-Length',
				value: expectedJson.length
			});

			expect(result).to.equal(expectedJson);
		});

		describe('override formatter per-object', () => {
			addBasicObjectFormattingTests((object, expectedJson, expectedStatusCode) => {
				// Arrange:
				const req = {};

				const resHeaders = [];
				const res = {
					setHeader: (key, value) => {
						resHeaders.push({ key, value });
					}
				};

				if (object)
					object.formatter = 'anotherFormatter';

				// Act:
				const result = createFormatters('anotherFormatter').json(req, res, object);

				// Assert:
				expect(res.statusCode).to.equal(expectedStatusCode);

				expect(resHeaders.length).to.equal(1);
				expect(resHeaders[0]).to.deep.equal({
					key: 'Content-Length',
					value: expectedJson.length
				});

				expect(result).to.equal(expectedJson);
			});
		});
	});

	describe('ws', () => {
		// note that formatters.ws ignores the status code
		addBasicObjectFormattingTests((object, expectedJson) => {
			// Act:
			const result = createFormatters('ws').ws(object);

			// Assert:
			expect(result).to.equal(expectedJson);
		});

		it('can bypass formatting of raw object', () => {
			// Arrange:
			const object = {
				payload: {
					foo: 123,
					bar: 987
				},
				type: 'raw'
			};

			// Act:
			const result = createFormatters('ws').ws(object);

			// Assert:
			expect(result).to.equal(object.payload);
		});
	});
});

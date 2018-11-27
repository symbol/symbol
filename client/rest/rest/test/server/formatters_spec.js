/*
 * Copyright (c) 2016-present,
 * Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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
	const createFormatters = name => {
		const formatUint64 = uint64 => (uint64 ? [uint64[0], uint64[1] * 2] : undefined);
		return formatters.create({
			[name]: {
				chainInfo: {
					format: chainInfo => ({
						height: formatUint64(chainInfo.height),
						scoreLow: formatUint64(chainInfo.scoreLow),
						scoreHigh: formatUint64(chainInfo.scoreHigh)
					})
				}
			}
		});
	};

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
					height: [1, 2],
					scoreLow: [112233, 8899],
					scoreHigh: [4, 3]
				},
				type: 'chainInfo'
			};

			// Assert: formatter doubles high part
			assertJsonFormat(object, '{"height":[1,4],"scoreLow":[112233,17798],"scoreHigh":[4,6]}', undefined);
		});

		it('can format catapult object array', () => {
			// Arrange:
			const object = {
				payload: [
					{ height: [1, 2] },
					{ height: [8, 7] }
				],
				type: 'chainInfo'
			};

			// Assert: formatter doubles high part
			assertJsonFormat(object, '[{"height":[1,4]},{"height":[8,14]}]', undefined);
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

				if (object) object.formatter = 'anotherFormatter';

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

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

import { sendJson, sendMetalData, sendPlainText } from '../../src/routes/simpleSend.js';
import { expect } from 'chai';

describe('simple send', () => {
	const addSendUnformattedTests = (senderFunction, validData, expectedContentType, expectedDescription) => {
		const createTestSetup = () => {
			const routeContext = { numNextCalls: 0 };
			const next = () => { ++routeContext.numNextCalls; };

			routeContext.responses = [];
			routeContext.headers = [];
			const res = {
				statusCode: 200,
				send: response => { routeContext.responses.push(response); },
				setHeader: (name, value) => { routeContext.headers.push({ name, value }); }
			};

			return { routeContext, next, res };
		};

		const runTest = (sendParams, assertResponse) => {
			// Arrange: set up the route params
			const { routeContext, next, res } = createTestSetup();
			const sender = senderFunction(res, next);

			// Act: send data
			sender(...sendParams);

			// Assert: exactly one response was sent
			expect(routeContext.numNextCalls).to.equal(1);
			expect(routeContext.responses.length).to.equal(1);
			assertResponse(routeContext.responses[0], res.statusCode, routeContext.headers);
		};

		it('fails when there is no data', () => {
			// Arrange:
			const { next, res } = createTestSetup();
			const sender = senderFunction(res, next);

			// Act + Assert:
			expect(() => sender(undefined)).to.throw(`error retrieving ${expectedDescription}`);
		});

		it('succeeds when there is data', () => {
			// Act: send valid data
			runTest([validData], (response, statusCode, headers) => {
				// Assert:
				expect(response).to.deep.equal(validData);
				expect(statusCode).to.equal(200);
				expect(headers).to.deep.equal([{ name: 'content-type', value: expectedContentType }]);
			});
		});

		it('succeeds when there is data, with custom status code', () => {
			// Act: send valid data
			runTest([validData, 502], (response, statusCode, headers) => {
				// Assert:
				expect(response).to.deep.equal(validData);
				expect(statusCode).to.equal(502);
				expect(headers).to.deep.equal([{ name: 'content-type', value: expectedContentType }]);
			});
		});
	};

	describe('send plain text', () => {
		addSendUnformattedTests(sendPlainText, 'HELLO world!', 'text/plain', 'plain text');
	});

	describe('send JSON object', () => {
		addSendUnformattedTests(sendJson, { foo: 'abc', bar: 'xyz' }, 'application/json', 'JSON object');
	});

	describe('send data', () => {
		const sendDataTest = (sender, assertResponse) => {
			// Arrange
			const routeContext = { numNextCalls: 0 };
			const next = () => { ++routeContext.numNextCalls; };
			const data = [];
			const headers = [];

			const res = {
				setHeader: (name, value) => {
					headers.push({ name, value });
				},
				write: writeData => {
					data.push(writeData);
				},
				end: () => {}
			};
			routeContext.responses = { headers, data };

			// Act: send the entity
			sender(res, next);

			expect(routeContext.numNextCalls).to.equal(1);
			assertResponse(routeContext.responses);
		};
		const send = ({
			data, mimeType, fileName, text, download
		}, assertResponse) => {
			sendDataTest((res, next) =>
				sendMetalData(res, next)(data, mimeType, fileName, text, download), assertResponse);
		};

		const assertHeader = (response, dataBuffer, disposition, mimeType, text) => {
			// Arrange:
			const contentTypeValue = response.headers.find(header => 'content-type' === header.name)?.value;
			const contentDispositionValue = response.headers.find(header => 'Content-Disposition' === header.name)?.value;
			const contentTextHeader = response.headers.find(header => 'Content-MetalText' === header.name);
			// Assert:
			expect(response.data.length).to.equal(1);
			expect(response.data[0]).to.deep.equal(dataBuffer);
			expect(contentDispositionValue).to.equal(disposition);
			expect(contentTypeValue).to.equal(mimeType);
			if (text)
				expect(contentTextHeader.value).to.equal(text);
			else
				expect(contentTextHeader).to.equal(undefined);
		};

		it('no params', () => {
			// Act:
			send({ data: Buffer.from([0, 1, 2]), mimeType: 'image/png' }, response => {
				assertHeader(response, Buffer.from([0, 1, 2]), 'inline;', 'image/png');
			});
		});
		it('with fileName', () => {
			// Act:
			send({ data: Buffer.from([0, 1, 2]), mimeType: 'image/png', fileName: 'image.png' }, response => {
				assertHeader(response, Buffer.from([0, 1, 2]), 'inline; filename="image.png"', 'image/png', undefined);
			});
		});
		it('with fileName and text', () => {
			// Act:
			send({
				data: Buffer.from([0, 1, 2]), mimeType: 'image/png', fileName: 'image.png', text: 'test'
			}, response => {
				assertHeader(response, Buffer.from([0, 1, 2]), 'inline; filename="image.png"', 'image/png', 'test');
			});
		});
		it('with fileName and text as attachment', () => {
			// Act:
			send({
				data: Buffer.from([0, 1, 2]), mimeType: 'image/png', fileName: 'image.png', text: 'test', download: 'true'
			}, response => {
				assertHeader(response, Buffer.from([0, 1, 2]), 'attachment; filename="image.png"', 'image/png', 'test');
			});
		});
		it('mimeType is application/octet-stream as attachment', () => {
			// Act:
			send({
				data: Buffer.from([0, 1, 2]), mimeType: 'application/octet-stream'
			}, response => {
				assertHeader(response, Buffer.from([0, 1, 2]), 'attachment;', 'application/octet-stream');
			});
		});
	});
});

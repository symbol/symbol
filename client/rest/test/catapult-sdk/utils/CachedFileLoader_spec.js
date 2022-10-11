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

const CachedFileLoader = require('../../../src/catapult-sdk/utils/CachedFileLoader');
const { expect } = require('chai');
const tmp = require('tmp');
const fs = require('fs');

describe('CachedFileLoader', () => {
	const createSplitProcessor = () => {
		let tag = 1;
		return contents => ([].concat(contents.split(' '), [tag++]));
	};
	const createErrorProcessor = () => (() => { throw Error('parse error'); });

	const addBasicTests = functionName => {
		it('loads new file', () => {
			// Arrange:
			const tempFile = tmp.fileSync();
			fs.writeFileSync(tempFile.name, 'NEM Symbol foo Bar');

			const loader = new CachedFileLoader();
			const processor = createSplitProcessor();

			// Act:
			return loader[functionName](tempFile.name, processor).then(data => {
				// Assert:
				expect(data).to.deep.equal(['NEM', 'Symbol', 'foo', 'Bar', 1]);
			});
		});

		it('fails when processor fails', () => {
			// Arrange:
			const tempFile = tmp.fileSync();
			fs.writeFileSync(tempFile.name, 'NEM Symbol foo Bar');

			const loader = new CachedFileLoader();
			const processor = createErrorProcessor();

			// Act + Assert:
			return loader[functionName](tempFile.name, processor).catch(err => {
				expect(() => { throw err; }).throws('parse error');
			});
		});

		it('fails when file does not exist', () => {
			// Arrange:
			const loader = new CachedFileLoader();
			const processor = createErrorProcessor();

			// Act + Assert:
			return loader[functionName]('foo.bar', processor).catch(err => {
				expect(() => { throw err; }).throws('no such file or directory');
			});
		});
	};

	const addReloadUnchangedFileTest = (functionName, expectedTag) => {
		it(`${1 < expectedTag ? 'reloads' : 'does not reload'} unchanged file`, () => {
			// Arrange:
			const tempFile = tmp.fileSync();
			fs.writeFileSync(tempFile.name, 'NEM Symbol foo Bar');

			const loader = new CachedFileLoader();
			const processor = createSplitProcessor();

			// Act:
			return loader[functionName](tempFile.name, processor)
				.then(() => loader[functionName](tempFile.name, processor))
				.then(data => {
					// Assert:
					expect(data).to.deep.equal(['NEM', 'Symbol', 'foo', 'Bar', expectedTag]);
				});
		});
	};

	const addReloadChangedFileTest = (functionName, expectedData) => {
		it(`${1 < expectedData[4] ? 'reloads' : 'does not reload'} changed file`, () => {
			// Arrange:
			const tempFile = tmp.fileSync();
			fs.writeFileSync(tempFile.name, 'NEM Symbol foo Bar');

			const loader = new CachedFileLoader();
			const processor = createSplitProcessor();

			// Act:
			return loader[functionName](tempFile.name, processor)
				// - slight pause before writing to make sure mtime is changed
				.then(() => new Promise(resolve => { setTimeout(resolve, 10); }))
				.then(() => {
					fs.writeFileSync(tempFile.name, 'XEM xym foo Bar');

					return loader[functionName](tempFile.name, processor).then(data => {
						// Assert:
						expect(data).to.deep.equal(expectedData);
					});
				});
		});
	};

	describe('readAlways', () => {
		addBasicTests('readAlways');
		addReloadUnchangedFileTest('readAlways', 2); // reloads
	});

	describe('readOnce', () => {
		addBasicTests('readOnce');
		addReloadUnchangedFileTest('readOnce', 1); // does not reload
		addReloadChangedFileTest('readOnce', ['NEM', 'Symbol', 'foo', 'Bar', 1]); // does not reload
	});

	describe('readNewer', () => {
		addBasicTests('readNewer');
		addReloadUnchangedFileTest('readNewer', 1); // does not reload
		addReloadChangedFileTest('readNewer', ['XEM', 'xym', 'foo', 'Bar', 2]); // reloads
	});

	it('supports multiple files in cache', () => {
		// Arrange:
		const tempFile1 = tmp.fileSync();
		fs.writeFileSync(tempFile1.name, 'NEM Symbol foo Bar');

		const tempFile2 = tmp.fileSync();
		fs.writeFileSync(tempFile2.name, 'lorem ipsum');

		const tempFile3 = tmp.fileSync();
		fs.writeFileSync(tempFile3.name, 'XEM xym foo Bar');

		const loader = new CachedFileLoader();
		const processor = contents => (contents.split(' '));

		// Act:
		return Promise.all([tempFile1, tempFile2, tempFile3].map(tempFile => loader.readAlways(tempFile.name, processor)))
			.then(() => Promise.all([tempFile1, tempFile2, tempFile3].map(tempFile => loader.readOnce(tempFile.name, processor))))
			.then(dataArray => {
				// Assert:
				expect(dataArray.length).to.equal(3);
				expect(dataArray[0]).to.deep.equal(['NEM', 'Symbol', 'foo', 'Bar']);
				expect(dataArray[1]).to.deep.equal(['lorem', 'ipsum']);
				expect(dataArray[2]).to.deep.equal(['XEM', 'xym', 'foo', 'Bar']);
			});
	});
});

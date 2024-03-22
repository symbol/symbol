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

const fs = require('fs');
const util = require('util');

const readFile = util.promisify(fs.readFile);
const stat = util.promisify(fs.stat);

/**
 * Loads and caches files.
 */
class CachedFileLoader {
	/**
	 * Creates a cached file loader.
	 */
	constructor() {
		this.cache = {};
	}

	/**
	 * Loads a file.
	 * @param {string} filePath Path to file to read.
	 * @param {Function} processor File processor.
	 * @returns {Promise} Promise to processed file contents.
	 */
	readAlways(filePath, processor) {
		return Promise.all([readFile(filePath), stat(filePath)])
			.then(results => {
				const textDecoder = new TextDecoder();
				const contents = textDecoder.decode(results[0]);
				const stats = results[1];

				this.cache[filePath] = {
					mtime: stats.mtime,
					data: processor(contents)
				};

				return this.cache[filePath].data;
			});
	}

	/**
	 * Loads a file exactly once and caches the result.
	 * @param {string} filePath Path to file to read.
	 * @param {Function} processor File processor.
	 * @returns {Promise} Promise to processed file contents.
	 */
	readOnce(filePath, processor) {
		return this.cache[filePath]
			? Promise.resolve(this.cache[filePath].data)
			: this.readAlways(filePath, processor);
	}

	/**
	 * Loads a file only if it has changed on disk since last load.
	 * @param {string} filePath Path to file to read.
	 * @param {Function} processor File processor.
	 * @returns {Promise} Promise to processed file contents.
	 */
	readNewer(filePath, processor) {
		if (!this.cache[filePath])
			return this.readAlways(filePath, processor);

		return stat(filePath)
			.then(stats => {
				// reprocess file if updated on disk
				if (stats.mtime > this.cache[filePath].mtime)
					return this.readAlways(filePath, processor);

				return this.cache[filePath].data;
			});
	}
}

module.exports = CachedFileLoader;

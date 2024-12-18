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

/** @module model/idReducer */

export default {
	/**
	 * Creates an id to name lookup object around namespace name tuples.
	 * @param {Array<object>} nameTuples Namespace name tuples.
	 * @returns {object} An id to name lookup object.
	 */
	createIdToNameLookup: nameTuples => {
		let nextRoundKeys = [];

		// copy all tuples into an id -> value dictionary
		const lookupMap = new Map();
		nameTuples.forEach(nameTuple => {
			const key = nameTuple.namespaceId;

			// give preference to first of conflicts
			if (!lookupMap.get(key)) {
				lookupMap.set(key, Object.assign({ fqn: nameTuple.name }, nameTuple));
				if (0n !== nameTuple.parentId) {
					lookupMap.get(key).nextId = nameTuple.parentId;
					nextRoundKeys.push(key);
				}
			}
		});

		// each round processes the tuples from the previous round that have a nonzero parent
		const processRoundKeys = roundKeys => {
			const additionalProcessingKeys = [];
			roundKeys.forEach(key => {
				const nameTuple = lookupMap.get(key);
				const parentEntry = lookupMap.get(nameTuple.nextId);
				nameTuple.fqn = parentEntry ? `${parentEntry.name}.${nameTuple.fqn}` : undefined;

				if (!parentEntry || 0n === parentEntry.parentId) {
					delete nameTuple.nextId;
				} else {
					// if the nextId is nonzero, additional processing is required
					nameTuple.nextId = parentEntry.parentId;
					additionalProcessingKeys.push(key);
				}
			});

			return additionalProcessingKeys;
		};

		while (0 !== nextRoundKeys.length) {
			const currentRoundKeys = nextRoundKeys.slice();
			nextRoundKeys = processRoundKeys(currentRoundKeys);
		}

		return {
			/**
			 * Number of id to name mappings known by this object.
			 * @type {number}
			 */
			length: nameTuples.length,

			/**
			 * Returns the name for an id or undefined if no mapping exists
			 * @param {bigint} id A bigint value representing a namespace id.
			 * @returns {string} Fully qualified namespace name corresponding to the id.
			 */
			findName: id => (lookupMap.get(id) || {}).fqn
		};
	}
};

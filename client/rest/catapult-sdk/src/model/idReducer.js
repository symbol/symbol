/** @module model/idReducer */
import uint64 from '../utils/uint64';

export default {
	/**
	 * Creates an id to name lookup object around namespace name tuples.
	 * @param {array} nameTuples The namespace name tuples.
	 * @returns {object} An id to name lookup object.
	 */
	createIdToNameLookup: nameTuples => {
		let nextRoundKeys = [];

		// copy all tuples into an id -> value dictionary
		const lookupMap = {};
		for (const nameTuple of nameTuples) {
			const key = uint64.toHex(nameTuple.namespaceId);

			// give preference to first of conflicts
			if (!lookupMap[key]) {
				lookupMap[key] = Object.assign({ fqn: nameTuple.name }, nameTuple);
				if (!uint64.isZero(nameTuple.parentId)) {
					lookupMap[key].nextId = nameTuple.parentId;
					nextRoundKeys.push(key);
				}
			}
		}

		// each round processes the tuples from the previous round that have a nonzero parent
		while (0 !== nextRoundKeys.length) {
			const currentRoundKeys = nextRoundKeys.slice();
			nextRoundKeys = [];

			for (const key of currentRoundKeys) {
				const nameTuple = lookupMap[key];
				const parentEntry = lookupMap[uint64.toHex(nameTuple.nextId)];
				nameTuple.fqn = parentEntry ? `${parentEntry.name}.${nameTuple.fqn}` : undefined;

				if (!parentEntry || uint64.isZero(parentEntry.parentId)) {
					delete nameTuple.nextId;
				} else {
					// if the nextId is nonzero, additional processing is required
					nameTuple.nextId = parentEntry.parentId;
					nextRoundKeys.push(key);
				}
			}
		}

		return {
			/**
			 * The number of id to name mappings known by this object.
			 * @property {numeric}
			 */
			length: nameTuples.length,

			/**
			 * Returns the name for an id or undefined if no mapping exists
			 * @param {module:utils/uint64~uint64} id A uint64 value representing a namespace id.
			 * @returns {string} The fully qualified namespace name corresponding to the id.
			 */
			findName: id => (lookupMap[uint64.toHex(id)] || {}).fqn
		};
	}
};

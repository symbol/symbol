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

const arrayUtils = require('../utils/arrayUtils');

const NodePosition = Object.freeze({
	left: 'left',
	right: 'right'
});

class HashNotFoundError extends Error {}
class InvalidTree extends Error {}

const evenify = number => (number % 2 ? number + 1 : number);

/**
 * Returns the index of a hash in a Merkle tree.
 * @param {Uint8Array} hash Hash to look up in the tree.
 * @param {object} tree Merkle tree object containing the number of hashed elements and the tree of hashes.
 * @returns {array} Index of the first element in the tree matching the given hash, otherwise -1 is returned.
 */
const indexOfLeafWithHash = (hash, tree) => tree.nodes
	.slice(0, evenify(tree.count))
	.findIndex(element => arrayUtils.deepEqual(element, hash));

const siblingOf = nodeIndex => {
	if (nodeIndex % 2) {
		return {
			position: NodePosition.left,
			index: nodeIndex - 1
		};
	}
	return {
		position: NodePosition.right,
		index: nodeIndex + 1
	};
};

/**
 * Given a Merkle tree and a hashed element in it, returns the audit path required for a consistency check.
 * @param {Uint8Array} hash Element's hash for which to build the audit path.
 * @param {object} tree Merkle tree object containing the number of elements and the tree of hashes.
 * @returns {array} Array of objects containing the Merkle tree hash, and its relative position (left or right).
 */
const buildAuditPath = (hash, tree) => {
	if (0 === tree.count)
		throw new InvalidTree();

	let layerStart = 0;
	let currentLayerCount = tree.count;
	let layerSubindexOfHash = indexOfLeafWithHash(hash, tree);
	if (-1 === layerSubindexOfHash)
		throw new HashNotFoundError();

	const auditPath = [];
	while (1 !== currentLayerCount) {
		currentLayerCount = evenify(currentLayerCount);
		const sibling = siblingOf(layerStart + layerSubindexOfHash);
		const siblingPathNode = {
			hash: tree.nodes[sibling.index],
			position: sibling.position
		};
		auditPath.push(siblingPathNode);
		layerStart += currentLayerCount;
		currentLayerCount /= 2;
		layerSubindexOfHash = Math.floor(layerSubindexOfHash / 2);
	}
	return auditPath;
};

module.exports = {
	buildAuditPath,
	evenify,
	indexOfLeafWithHash,
	siblingOf,
	NodePosition,
	HashNotFoundError,
	InvalidTree
};

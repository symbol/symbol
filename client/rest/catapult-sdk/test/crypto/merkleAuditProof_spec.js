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

const convert = require('../../src/utils/convert');
const {
	indexOfLeafWithHash, buildAuditPath, NodePosition, siblingOf, HashNotFoundError,
	InvalidTree, evenify
} = require('../../src/crypto/merkleAuditProof.js');
const { expect } = require('chai');

const hexStringToBuffer = input => Buffer.from(convert.hexToUint8(input), 'hex');

const merkleTree = {
	count: 4,
	nodes: [
		hexStringToBuffer('8D25B2639A7D12FEAAAEF34358B215E97533F9FFDDA5B9FADFD8ECC229695263'),
		hexStringToBuffer('8AB2F19A47C5B30CC389AE1580F0472B4D3AFEEA83CDF0F707D03ED76B15A00C'),
		hexStringToBuffer('B9840C4EADB6724A2DFCA81D5E90EF3F4EE91BEB63A58FA91A4F05E951F08FCF'),
		hexStringToBuffer('B9840C4EADB6724A2DFCA81D5E90EF3F4EE91BEB63A58FA91A4F05E951F08FCF'),
		hexStringToBuffer('586B203612B0E5CA695CFF677A5B784E4368B79C1A7B272036105753A915EDC9'),
		hexStringToBuffer('AA1DFFF01B3ABA492188195DF4D77AF25FF9D57DF4EA0FD6FE498D572B6E67FD'),
		hexStringToBuffer('16D28DD7AF87B86C79273450470E96F22C66F8EF4598064603699B69F10464D0')
	]
};

const merkleTreeLong = {
	count: 6,
	nodes: [
		hexStringToBuffer('A983745F69959AF438C5B59501B7B6FCD4312DE1F5252A6E8B54D09E23266A7C'),
		hexStringToBuffer('5A639C5865FFA3331C3315BE2797F490D7D9C12826AF08C3643929DCAC391E21'),
		hexStringToBuffer('F95D66CCB9B788A582FB35ABB4328BD705E2DB97D3F2EFAED46C88584A87E202'),
		hexStringToBuffer('C11A995692E24FE9A79B657375AA051D97B5261C14A2F08792AE5269F412BD8F'),
		hexStringToBuffer('0CDD981B532DDB0789A3B7F96B37FA0973EE6EB1D41FA8AA8E0411BA2FB07851'),
		hexStringToBuffer('C7F87E9FC96202ECE5F219EA87A2C02363B763520A1D7B08F0D2037FFF7CC1E0'),
		hexStringToBuffer('5EEEF952CE75555C24C4820E7BE36370B0AD9ABFE357D4244AC1DA1E1102229A'),
		hexStringToBuffer('EFAE07E3096428E93611072581F769F12758345BEE2779A3B20326F8A1A6C373'),
		hexStringToBuffer('DF646700D4CDBA8DF803EE27F0E8DE59A32AD95E74803C342431F1234E9D054C'),
		hexStringToBuffer('DF646700D4CDBA8DF803EE27F0E8DE59A32AD95E74803C342431F1234E9D054C'),
		hexStringToBuffer('9ED4FE218563EE52D2AE18CAFD9CD12A403EAD9737EFC1B8AADD892A87699AB5'),
		hexStringToBuffer('11E903589FAE58D8DDB460F11F33A80A9B2CF09D4B50FCC615C5440337CEBE4F'),
		hexStringToBuffer('F1BDD998E8C54C8B71CEC7B9AAC14E3A0B93F2EC93E445542885F29DA5375787')
	]
};

describe('evenify', () => {
	it('should return the valid even number', () => {
		expect(evenify(0)).to.equal(0);
		expect(evenify(1)).to.equal(2);
		expect(evenify(2)).to.equal(2);
		expect(evenify(3)).to.equal(4);
		expect(evenify(13)).to.equal(14);
		expect(evenify(14)).to.equal(14);
	});
});

describe('indexOfLeafWithHash', () => {
	it('should return -1 if hash not found in tree', () => {
		expect(indexOfLeafWithHash(hexStringToBuffer('9ED4FE218563EE52D2AE18CAFD9CD12A403EAD9737EFC1B8AADD892A00000000'), merkleTree))
			.to.equal(-1);
	});

	it('should return -1 if hash not found in tree leaves', () => {
		expect(indexOfLeafWithHash(merkleTree.nodes[4], merkleTree)).to.equal(-1);
	});

	it('should return -1 if hash not found in an empty tree', () => {
		expect(indexOfLeafWithHash(
			hexStringToBuffer('9ED4FE218563EE52D2AE18CAFD9CD12A403EAD9737EFC1B8AADD892A00000000'),
			{ nodes: [], count: 0 }
		)).to.equal(-1);
	});

	it('should return the transaction found for a single node tree', () => {
		const hash = hexStringToBuffer('9ED4FE218563EE52D2AE18CAFD9CD12A403EAD9737EFC1B8AADD892A00000000');
		expect(indexOfLeafWithHash(
			hash,
			{ nodes: [hash], count: 1 }
		)).to.equal(0);
	});

	it('should return the index for a found node', () => {
		expect(indexOfLeafWithHash(merkleTree.nodes[2], merkleTree)).to.equal(2);
	});

	it('should return the index for a found node in the first leaf', () => {
		expect(indexOfLeafWithHash(merkleTree.nodes[0], merkleTree)).to.equal(0);
	});

	it('should return the index for a found duplicated node in the last leaf out of the number of transactions', () => {
		// Arrange:
		const merkleTree2 = {
			count: 3,
			nodes: [
				hexStringToBuffer('8D25B2639A7D12FEAAAEF34358B215E97533F9FFDDA5B9FADFD8ECC229695263'),
				hexStringToBuffer('8AB2F19A47C5B30CC389AE1580F0472B4D3AFEEA83CDF0F707D03ED76B15A00C'),
				hexStringToBuffer('B9840C4EADB6724A2DFCA81D5E90EF3F4EE91BEB63A58FA91A4F05E951F08FCF'),
				hexStringToBuffer('B9840C4EADB6724A2DFCA81D5E90EF3F4EE91BEB63A58FA91A4F05E951F08FCF'),
				hexStringToBuffer('586B203612B0E5CA695CFF677A5B784E4368B79C1A7B272036105753A915EDC9'),
				hexStringToBuffer('AA1DFFF01B3ABA492188195DF4D77AF25FF9D57DF4EA0FD6FE498D572B6E67FD'),
				hexStringToBuffer('16D28DD7AF87B86C79273450470E96F22C66F8EF4598064603699B69F10464D0')
			]
		};

		// Assert:
		expect(indexOfLeafWithHash(merkleTree2.nodes[2], merkleTree2)).to.equal(2);
	});
});

describe('siblingOf', () => {
	it('should return valid value if first index', () => {
		expect(siblingOf(0).index).to.equal(1);
	});

	it('should return valid values on different tree levels', () => {
		expect(siblingOf(1)).to.deep.equal({ position: NodePosition.left, index: 0 });
		expect(siblingOf(2)).to.deep.equal({ position: NodePosition.right, index: 3 });
		expect(siblingOf(13)).to.deep.equal({ position: NodePosition.left, index: 12 });
		expect(siblingOf(14)).to.deep.equal({ position: NodePosition.right, index: 15 });
	});
});

describe('buildAuditPath', () => {
	it('should return an empty audit proof trail for a single node tree', () => {
		const trail = buildAuditPath(merkleTree.nodes[0], { nodes: [merkleTree.nodes[0]], count: 1 });
		expect(trail.length).to.equal(0);
	});

	it('should throw if hash not in tree', () => {
		expect(() => buildAuditPath(hexStringToBuffer('000903589FAE58D8DDB460F11F33A80A9B2CF09D4B50FCC615C5440337CEBE4F'), merkleTree))
			.to.throw(HashNotFoundError);
	});

	it('should throw if tree is empty', () => {
		expect(() => buildAuditPath(
			hexStringToBuffer('000903589FAE58D8DDB460F11F33A80A9B2CF09D4B50FCC615C5440337CEBE4F'),
			{ nodes: [], count: 0 }
		)).to.throw(InvalidTree);
	});

	it('should throw if hash in tree but not leaf', () => {
		expect(() => buildAuditPath(
			merkleTree.nodes[4],
			merkleTree
		)).to.throw(HashNotFoundError);
		expect(() => buildAuditPath(
			merkleTree.nodes[6],
			merkleTree
		)).to.throw(HashNotFoundError);
	});

	it('should return correctly if number of transactions is not even', () => {
		// Arrange:
		const merkleTree2 = {
			count: 3,
			nodes: [
				hexStringToBuffer('8D25B2639A7D12FEAAAEF34358B215E97533F9FFDDA5B9FADFD8ECC229695263'),
				hexStringToBuffer('8AB2F19A47C5B30CC389AE1580F0472B4D3AFEEA83CDF0F707D03ED76B15A00C'),
				hexStringToBuffer('B9840C4EADB6724A2DFCA81D5E90EF3F4EE91BEB63A58FA91A4F05E951F08FCF'),
				hexStringToBuffer('B9840C4EADB6724A2DFCA81D5E90EF3F4EE91BEB63A58FA91A4F05E951F08FCF'),
				hexStringToBuffer('586B203612B0E5CA695CFF677A5B784E4368B79C1A7B272036105753A915EDC9'),
				hexStringToBuffer('AA1DFFF01B3ABA492188195DF4D77AF25FF9D57DF4EA0FD6FE498D572B6E67FD'),
				hexStringToBuffer('16D28DD7AF87B86C79273450470E96F22C66F8EF4598064603699B69F10464D0')
			]
		};

		// Assert:
		const trail0 = buildAuditPath(merkleTree2.nodes[0], merkleTree2);
		expect(trail0).to.deep.equal([
			{ hash: merkleTree2.nodes[1], position: 2 },
			{ hash: merkleTree2.nodes[5], position: 2 }
		]);

		const trail1 = buildAuditPath(merkleTree2.nodes[1], merkleTree2);
		expect(trail1).to.deep.equal([
			{ hash: merkleTree2.nodes[0], position: 1 },
			{ hash: merkleTree2.nodes[5], position: 2 }
		]);

		const trail2 = buildAuditPath(merkleTree2.nodes[2], merkleTree2);
		expect(trail2).to.deep.equal([
			{ hash: merkleTree2.nodes[3], position: 2 },
			{ hash: merkleTree2.nodes[4], position: 1 }
		]);
	});

	it('should return correctly if number of transactions is even', () => {
		const trail0 = buildAuditPath(merkleTree.nodes[0], merkleTree);
		expect(trail0).to.deep.equal([
			{ hash: merkleTree.nodes[1], position: 2 },
			{ hash: merkleTree.nodes[5], position: 2 }
		]);

		const trail1 = buildAuditPath(merkleTree.nodes[1], merkleTree);
		expect(trail1).to.deep.equal([
			{ hash: merkleTree.nodes[0], position: 1 },
			{ hash: merkleTree.nodes[5], position: 2 }
		]);

		const trail2 = buildAuditPath(merkleTree.nodes[2], merkleTree);
		expect(trail2).to.deep.equal([
			{ hash: merkleTree.nodes[3], position: 2 },
			{ hash: merkleTree.nodes[4], position: 1 }
		]);

		const trail3 = buildAuditPath(merkleTree.nodes[3], merkleTree);
		expect(trail3).to.deep.equal([
			{ hash: merkleTree.nodes[2], position: 2 },
			{ hash: merkleTree.nodes[4], position: 1 }
		]);
	});

	it('should return correctly if number of transactions is even on a four-level tree', () => {
		const trail0 = buildAuditPath(merkleTreeLong.nodes[0], merkleTreeLong);
		expect(trail0).to.deep.equal([
			{ hash: merkleTreeLong.nodes[1], position: 2 },
			{ hash: merkleTreeLong.nodes[7], position: 2 },
			{ hash: merkleTreeLong.nodes[11], position: 2 }
		]);

		const trail1 = buildAuditPath(merkleTreeLong.nodes[1], merkleTreeLong);
		expect(trail1).to.deep.equal([
			{ hash: merkleTreeLong.nodes[0], position: 1 },
			{ hash: merkleTreeLong.nodes[7], position: 2 },
			{ hash: merkleTreeLong.nodes[11], position: 2 }
		]);

		const trail2 = buildAuditPath(merkleTreeLong.nodes[2], merkleTreeLong);
		expect(trail2).to.deep.equal([
			{ hash: merkleTreeLong.nodes[3], position: 2 },
			{ hash: merkleTreeLong.nodes[6], position: 1 },
			{ hash: merkleTreeLong.nodes[11], position: 2 }
		]);

		const trail3 = buildAuditPath(merkleTreeLong.nodes[3], merkleTreeLong);
		expect(trail3).to.deep.equal([
			{ hash: merkleTreeLong.nodes[2], position: 1 },
			{ hash: merkleTreeLong.nodes[6], position: 1 },
			{ hash: merkleTreeLong.nodes[11], position: 2 }
		]);

		const trail4 = buildAuditPath(merkleTreeLong.nodes[4], merkleTreeLong);
		expect(trail4).to.deep.equal([
			{ hash: merkleTreeLong.nodes[5], position: 2 },
			{ hash: merkleTreeLong.nodes[9], position: 2 },
			{ hash: merkleTreeLong.nodes[10], position: 1 }
		]);

		const trail5 = buildAuditPath(merkleTreeLong.nodes[5], merkleTreeLong);
		expect(trail5).to.deep.equal([
			{ hash: merkleTreeLong.nodes[4], position: 1 },
			{ hash: merkleTreeLong.nodes[9], position: 2 },
			{ hash: merkleTreeLong.nodes[10], position: 1 }
		]);
	});
});

describe('NodePosition', () => {
	it('should be 1 for Left value', () => {
		expect(NodePosition.left).to.equal(1);
	});

	it('should be 2 for Right value', () => {
		expect(NodePosition.right).to.equal(2);
	});
});

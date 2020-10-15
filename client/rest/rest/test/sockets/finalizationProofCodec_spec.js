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

const finalizationProofCodec = require('../../src/sockets/finalizationProofCodec');
const catapult = require('catapult-sdk');
const { expect } = require('chai');

const { BinaryParser } = catapult.parser;

describe('deserialize', () => {
	const size = Buffer.from([0x38, 0x00, 0x00, 0x00]); // 4b
	const version = Buffer.from([0x64, 0x00, 0x00, 0x00]); // 4b
	const finalizationEpoch = Buffer.from([0x02, 0x00, 0x00, 0x00]); // 4b
	const finalizationPoint = Buffer.from([0x01, 0x00, 0x00, 0x00]); // 4b
	const height = Buffer.from([0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]); // 8b
	const hash = Buffer.from([ // 32b
		0xC3, 0xC4, 0xDE, 0xA0, 0xDA, 0xBD, 0x5C, 0xFA, 0x0D, 0x4B, 0x94, 0x1D, 0x15, 0xBB, 0x51, 0xB1,
		0xB4, 0x64, 0xBB, 0x00, 0xFF, 0x11, 0xFF, 0x00, 0x9F, 0xD0, 0x9A, 0x8F, 0x3D, 0x35, 0xF8, 0xF3
	]);
	const testPublicKey = Buffer.from([ // 32b
		0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB,
		0xEE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF
	]);
	const testInnerSignature = Buffer.from([ // 64b
		0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x99,
		0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB,
		0xCC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDD,
		0xEE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF
	]);

	const createSignature = () => {
		const rootParentPublicKey = testPublicKey;
		const rootSignature = testInnerSignature;
		const topParentPublicKey = testPublicKey;
		const topSignature = testInnerSignature;
		const bottomParentPublicKey = testPublicKey;
		const bottomSignature = testInnerSignature;

		const signature = [
			rootParentPublicKey, rootSignature,
			topParentPublicKey, topSignature,
			bottomParentPublicKey, bottomSignature
		];

		return Buffer.concat(signature);
	};

	const createMessageGroup = (hashCount, signatureCount) => {
		const messageGroupSize = Buffer.from([0x00, 0x00, 0x00, 0x00]); // 4b
		const messageGrouphashCount = Buffer.from([0x00, 0x00, 0x00, 0x00]); // 4b
		const messageGroupsignatureCount = Buffer.from([0x00, 0x00, 0x00, 0x00]); // 4b
		const messageGroupStage = Buffer.from([0x01, 0x00, 0x00, 0x00]); // 4b
		const messageGroupHeight = Buffer.from([0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]); // 8b

		const messageGroup = [
			messageGroupSize, messageGrouphashCount, messageGroupsignatureCount, messageGroupStage, messageGroupHeight
		];

		for (let i = 0; i < hashCount; ++i)
			messageGroup.push(hash);

		for (let i = 0; i < signatureCount; ++i)
			messageGroup.push(createSignature());

		const messageGroupBuffer = Buffer.concat(messageGroup);
		messageGroupBuffer.writeInt32LE(messageGroupBuffer.length, 0);
		messageGroupBuffer.writeInt32LE(hashCount, 4);
		messageGroupBuffer.writeInt32LE(signatureCount, 8);

		return messageGroupBuffer;
	};

	const runFinalizationProofTests = (testDescription, messageGroupsCount, hashCount, signatureCount) =>
		it(testDescription, () => {
			// Arrange:
			const expectedHashes = [];
			for (let i = 0; i < hashCount; ++i)
				expectedHashes.push(hash);

			const expectedSignatures = [];
			for (let i = 0; i < signatureCount; ++i) {
				expectedSignatures.push({
					root: {
						parentPublicKey: testPublicKey,
						signature: testInnerSignature
					},
					top: {
						parentPublicKey: testPublicKey,
						signature: testInnerSignature
					},
					bottom: {
						parentPublicKey: testPublicKey,
						signature: testInnerSignature
					}
				});
			}

			const finalizationProof = [
				size, version, finalizationEpoch, finalizationPoint, height, hash
			];

			const expectedMessageGroups = [];
			for (let i = 0; i < messageGroupsCount; ++i) {
				finalizationProof.push(createMessageGroup(hashCount, signatureCount));
				expectedMessageGroups.push({
					stage: 1,
					height: [1, 0],
					hashes: expectedHashes,
					signatures: expectedSignatures
				});
			}

			const finalizationProofBuffer = Buffer.concat(finalizationProof);
			finalizationProofBuffer.writeInt32LE(finalizationProofBuffer.length, 0);

			const binaryParser = new BinaryParser();
			binaryParser.push(Buffer.from(finalizationProofBuffer));

			// Assert:
			expect(finalizationProofCodec.deserialize(binaryParser)).to.deep.equal({
				version: 100,
				finalizationEpoch: 2,
				finalizationPoint: 1,
				height: [10, 0],
				hash,
				messageGroups: expectedMessageGroups
			});
		});

	it('returns undefined if object is 0 bytes length', () => {
		// Arrange:
		const binaryParser = new BinaryParser();
		const packet = [];
		binaryParser.push(Buffer.from(packet));

		// Assert:
		expect(finalizationProofCodec.deserialize(binaryParser)).to.equal(undefined);
	});

	runFinalizationProofTests('returns a deserialized finalization proof object with no message groups', 0, 0, 0);

	describe('returns a deserialized finalization proof object with message groups', () => {
		runFinalizationProofTests('with one message group', 1, 0, 0);
		runFinalizationProofTests('with multiple message group', 3, 0, 0);

		describe('message group with hashes', () => {
			runFinalizationProofTests('with one hash', 1, 1, 0);
			runFinalizationProofTests('with multiple hash', 1, 3, 0);
		});

		describe('message group with signatures', () => {
			runFinalizationProofTests('with one signature', 1, 0, 1);
			runFinalizationProofTests('with multiple signatures', 1, 0, 3);
		});

		describe('message group with hashes and signatures', () => {
			runFinalizationProofTests('with one of each', 1, 1, 1);
			runFinalizationProofTests('with multiple of each', 1, 3, 3);
		});
	});
});

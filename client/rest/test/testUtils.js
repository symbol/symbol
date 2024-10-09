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

import MongoDb from 'mongodb';
import { Hash256, PublicKey, Signature } from 'symbol-sdk';
import { Address } from 'symbol-sdk/symbol';
import winston from 'winston';
import crypto from 'crypto';

const random = {
	bytes: size => crypto.randomBytes(size),
	publicKey: () => crypto.randomBytes(PublicKey.SIZE),
	hash: () => crypto.randomBytes(Hash256.SIZE),
	secret: () => crypto.randomBytes(Hash256.SIZE),
	signature: () => crypto.randomBytes(Signature.SIZE),
	address: () => crypto.randomBytes(Address.SIZE),
	account: () => ({
		publicKey: random.publicKey(),
		address: random.address()
	})
};

export default {
	random,
	factory: {
		createBinary: buffer => new MongoDb.Binary(buffer),
		createObjectIdFromHexString: id => new MongoDb.ObjectId(id)
	},
	log: (...args) => {
		winston.debug(...args);
	},
	createLogger: () => winston,
	createMockLogger: () => {
		const logger = {};
		logger.numLogs = 0;
		['debug', 'info', 'warn', 'error'].forEach(level => {
			logger[level] = () => { ++logger.numLogs; };
		});
		return logger;
	},
	createSampleBlock: () => ({
		buffer: Buffer.concat([
			Buffer.of(0x78, 0x01, 0x00, 0x00), // size 4b
			Buffer.of(0x00, 0x00, 0x00, 0x00), // verifiable entity header reserved 1 4b
			Buffer.from(
				'3B98FF9829881B5D0218C2532741DFD4609BA64A503193A9AA35D35A05FED163'
					+ 'C721920D99ECBF3A4713270494AEAA2F893BEC49BE87995BE3671FC61EE5520B',
				'hex'
			), // signature 64b
			Buffer.from('A4C656B45C02A02DEF64F15DD781DD5AF29698A353F414FAAA9CDB364A09F98F', 'hex'), // signerPublicKey 32b
			Buffer.of(0x00, 0x00, 0x00, 0x00), // entity body reserved 1 4b
			Buffer.of(0x03), // version 1b
			Buffer.of(0x68), // network 1b
			Buffer.of(0x43, 0x81), // type 2b
			Buffer.of(0x97, 0x87, 0x45, 0x0E, 0xE1, 0x6C, 0xB6, 0x62), // height 8b
			Buffer.of(0xCB, 0x6C, 0x7E, 0x9A, 0x50, 0x20, 0x6F, 0xE7), // timestamp 8b
			Buffer.of(0x70, 0xC4, 0x32, 0xC5, 0x48, 0x2E, 0xC0, 0x10), // difficulty 8b
			Buffer.from('C6B916F4521C1114F3D6C12E152399A1F3B817C35C451F2281A166A73BCFAEC4', 'hex'), // proofGamma 32b
			Buffer.from('8430FC3A2BF8B047D2D43B5857C3FAE0', 'hex'), // proofVerificationHash 16b
			Buffer.from('81E1E6F9D5F1A9E8C43BD1A561E739D23077516D751D65242B36370498B5E58E', 'hex'), // proofScalar 32b
			Buffer.from('AFB52714878340832850280A6E85880A3A57A5A5A67300C2D01B0F5CF5095EF8', 'hex'), // previous block hash 32b
			Buffer.from('175D5E74BCDA4D69BC13297D7777E883A13D413299791EC303EE4E7E048F7EE3', 'hex'), // transactionsHashBuffer 32b
			Buffer.from('678E1529474BD842E22DB5647B31A28F304BD9471D88FB78B86D2FE72A11ADC7', 'hex'), // receiptsHashBuffer 32b
			Buffer.from('A416B7902DF91E26900494B5DA3EB5F918A9DD25685CF09183A3F4457095F045', 'hex'), // stateHashBuffer 32b
			Buffer.from('464CE24A140FA60D8C3B8DCF747BB5CE47E15FD70BCF3BB9', 'hex'), // beneficiaryAddress 24b
			Buffer.of(0x0A, 0x00, 0x00, 0x00), // fee feeMultiplierBuffer 4b
			Buffer.of(0x00, 0x00, 0x00, 0x00) // reserved padding 4b
		]),
		model: {
			signature: '3B98FF9829881B5D0218C2532741DFD4609BA64A503193A9AA35D35A05FED163'
				+ 'C721920D99ECBF3A4713270494AEAA2F893BEC49BE87995BE3671FC61EE5520B',
			signerPublicKey: 'A4C656B45C02A02DEF64F15DD781DD5AF29698A353F414FAAA9CDB364A09F98F',
			version: 3,
			network: 104,
			type: 33091,
			height: '7112992375341156247',
			timestamp: '16676583475737685195',
			difficulty: '1207015590216254576',
			// flatten generationHashProof because this is REST JSON
			proofGamma: 'C6B916F4521C1114F3D6C12E152399A1F3B817C35C451F2281A166A73BCFAEC4',
			proofVerificationHash: '8430FC3A2BF8B047D2D43B5857C3FAE0',
			proofScalar: '81E1E6F9D5F1A9E8C43BD1A561E739D23077516D751D65242B36370498B5E58E',
			previousBlockHash: 'AFB52714878340832850280A6E85880A3A57A5A5A67300C2D01B0F5CF5095EF8',
			transactionsHash: '175D5E74BCDA4D69BC13297D7777E883A13D413299791EC303EE4E7E048F7EE3',
			receiptsHash: '678E1529474BD842E22DB5647B31A28F304BD9471D88FB78B86D2FE72A11ADC7',
			stateHash: 'A416B7902DF91E26900494B5DA3EB5F918A9DD25685CF09183A3F4457095F045',
			beneficiaryAddress: '464CE24A140FA60D8C3B8DCF747BB5CE47E15FD70BCF3BB9',
			feeMultiplier: 10,
			transactions: []
		}
	}),
	createSampleTransaction: () => ({
		buffer: Buffer.concat([
			Buffer.of(0x91, 0x00, 0x00, 0x00), // size 4b
			Buffer.of(0x00, 0x00, 0x00, 0x00), // verifiable entity header reserved 1 4b
			Buffer.from(
				'40911560C84EEFBCFB41D0AB8767B19C41398AD699010CD82586602E3446B8F5'
					+ '6F8D259D65B28D65CCEA72A289B63F179B6C89E13A52B9B7C182CD4EEC5F9CD4',
				'hex'
			), // signature 64b
			Buffer.from('A571A3FD7FB5A312CA3A83097B993F67F8F14B7300885F7E4FEE2CAC86E7BB8D', 'hex'), // signerPublicKey 32b
			Buffer.of(0x00, 0x00, 0x00, 0x00), // entity body reserved 1 4b
			Buffer.of(0x01), // version 1b
			Buffer.of(0x68), // network 1b
			Buffer.of(0x4D, 0x42), // type 2b (mosaic supply change)
			Buffer.of(0x68, 0xF7, 0x05, 0x8D, 0xAC, 0xC8, 0x32, 0x6B), // fee 8b
			Buffer.of(0x26, 0xB6, 0x01, 0x63, 0x2F, 0x90, 0x47, 0xDC), // deadline 8b
			Buffer.of(0x51, 0x5E, 0xD0, 0xFA, 0xDB, 0xD2, 0xA4, 0xED), // mosaicId 8b
			Buffer.of(0x8E, 0x55, 0x1E, 0xAB, 0xC8, 0x8A, 0x73, 0xEC), // delta 8b
			Buffer.of(0x01) // action 1b
		]),
		model: {
			signature: '40911560C84EEFBCFB41D0AB8767B19C41398AD699010CD82586602E3446B8F5'
				+ '6F8D259D65B28D65CCEA72A289B63F179B6C89E13A52B9B7C182CD4EEC5F9CD4',
			signerPublicKey: 'A571A3FD7FB5A312CA3A83097B993F67F8F14B7300885F7E4FEE2CAC86E7BB8D',
			version: 1,
			network: 104,
			type: 16973,
			fee: '7724456954319730536',
			deadline: '15872813944889521702',
			mosaicId: '17124043525417098833',
			delta: '17038114409741702542',
			action: 1
		}
	})
};

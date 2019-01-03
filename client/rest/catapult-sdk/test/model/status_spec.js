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

const status = require('../../src/model/status');
const { expect } = require('chai');

describe('status', () => {
	describe('toString', () => {
		it('can output well-known enum values', () => {
			// Assert:
			expect(status.toString(0x00000000)).to.equal('Success');
			expect(status.toString(0x40000000)).to.equal('Neutral');
			expect(status.toString(0x80000000)).to.equal('Failure');
		});

		it('can output known plugin enum values', () => {
			// Assert:
			expect(status.toString(0x80410003)).to.equal('Failure_Aggregate_Too_Many_Cosignatures');
			expect(status.toString(0x80FF0066)).to.equal('Failure_Chain_Unlinked');
			expect(status.toString(0x80FE2002)).to.equal('Failure_Consumer_Remote_Chain_Improper_Link');
			expect(status.toString(0x80430005)).to.equal('Failure_Core_Insufficient_Balance');
			expect(status.toString(0x80450101)).to.equal('Failure_Extension_Partial_Transaction_Cache_Prune');
			expect(status.toString(0x81480007)).to.equal('Failure_Hash_Exists');
			expect(status.toString(0x80520009)).to.equal('Failure_LockSecret_Invalid_Duration');
			expect(status.toString(0x804D0002)).to.equal('Failure_Mosaic_Invalid_Name');
			expect(status.toString(0x80550003)).to.equal('Failure_Multisig_Modify_Redundant_Modifications');
			expect(status.toString(0x804E0002)).to.equal('Failure_Namespace_Invalid_Name');
			expect(status.toString(0x80530008)).to.equal('Failure_Signature_Not_Verifiable');
			expect(status.toString(0x80540006)).to.equal('Failure_Transfer_Message_Too_Large');
		});

		it('can output unknown enum values', () => {
			// Assert:
			expect(status.toString(0xABCD9812)).to.equal('unknown status 0xABCD9812');
			expect(status.toString(0x00CD9812)).to.equal('unknown status 0x00CD9812');
		});
	});
});

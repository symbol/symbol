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

/** @module model/status */

/* istanbul ignore next */
const toStringInternal = code => {
	switch (code) {
	case 0x00000000: return 'Success';
	case 0x40000000: return 'Neutral';
	case 0x80000000: return 'Failure';
	case 0x80430003: return 'Failure_Core_Past_Deadline';
	case 0x80430004: return 'Failure_Core_Future_Deadline';
	case 0x80430005: return 'Failure_Core_Insufficient_Balance';
	case 0x8043000A: return 'Failure_Core_Timestamp_Too_Far_In_Future';
	case 0x8043000F: return 'Failure_Core_Too_Many_Transactions';
	case 0x80430012: return 'Failure_Core_Nemesis_Account_Signed_After_Nemesis_Block';
	case 0x80430014: return 'Failure_Core_Wrong_Network';
	case 0x80430015: return 'Failure_Core_Invalid_Address';
	case 0x80430069: return 'Failure_Core_Block_Harvester_Ineligible';
	case 0x81480007: return 'Failure_Hash_Exists';
	case 0x80530008: return 'Failure_Signature_Not_Verifiable';
	case 0x80410001: return 'Failure_Aggregate_Too_Many_Transactions';
	case 0x80410002: return 'Failure_Aggregate_No_Transactions';
	case 0x80410003: return 'Failure_Aggregate_Too_Many_Cosignatures';
	case 0x80410004: return 'Failure_Aggregate_Redundant_Cosignatures';
	case 0x80411001: return 'Failure_Aggregate_Ineligible_Cosigners';
	case 0x80411002: return 'Failure_Aggregate_Missing_Cosigners';
	case 0x804C0001: return 'Failure_Lock_Invalid_Hash_Algorithm';
	case 0x804C0002: return 'Failure_Lock_Invalid_Mosaic_Id';
	case 0x804C0003: return 'Failure_Lock_Invalid_Mosaic_Amount';
	case 0x804C0004: return 'Failure_Lock_Hash_Exists';
	case 0x804C0005: return 'Failure_Lock_Hash_Does_Not_Exist';
	case 0x804C0006: return 'Failure_Lock_Missing_Secret_Padding';
	case 0x804C0007: return 'Failure_Lock_Hash_Not_Implemented';
	case 0x804C0008: return 'Failure_Lock_Proof_Size_Out_Of_Bounds';
	case 0x804C0009: return 'Failure_Lock_Secret_Mismatch';
	case 0x804C000A: return 'Failure_Lock_Unknown_Secret';
	case 0x804C000B: return 'Failure_Lock_Inactive_Hash';
	case 0x804C000C: return 'Failure_Lock_Inactive_Secret';
	case 0x804C000D: return 'Failure_Lock_Hash_Algorithm_Mismatch';
	case 0x804C000E: return 'Failure_Lock_Hash_Already_Used';
	case 0x804C000F: return 'Failure_Lock_Secret_Already_Used';
	case 0x804C0010: return 'Failure_Lock_Invalid_Duration';
	case 0x80550001: return 'Failure_Multisig_Modify_Account_In_Both_Sets';
	case 0x80550002: return 'Failure_Multisig_Modify_Multiple_Deletes';
	case 0x80550003: return 'Failure_Multisig_Modify_Redundant_Modifications';
	case 0x80550004: return 'Failure_Multisig_Modify_Unknown_Multisig_Account';
	case 0x80550005: return 'Failure_Multisig_Modify_Not_A_Cosigner';
	case 0x80550006: return 'Failure_Multisig_Modify_Already_A_Cosigner';
	case 0x80550007: return 'Failure_Multisig_Modify_Min_Setting_Out_Of_Range';
	case 0x80550008: return 'Failure_Multisig_Modify_Min_Setting_Larger_Than_Num_Cosignatories';
	case 0x80550009: return 'Failure_Multisig_Modify_Unsupported_Modification_Type';
	case 0x8055000A: return 'Failure_Multisig_Modify_Max_Cosigned_Accounts';
	case 0x8055000B: return 'Failure_Multisig_Modify_Max_Cosigners';
	case 0x8055000C: return 'Failure_Multisig_Modify_Loop';
	case 0x8055000D: return 'Failure_Multisig_Modify_Max_Multisig_Depth';
	case 0x80550800: return 'Failure_Multisig_Operation_Not_Permitted_By_Account';
	case 0x804E0001: return 'Failure_Namespace_Invalid_Duration';
	case 0x804D0001: return 'Failure_Mosaic_Invalid_Duration';
	case 0x804E0002: return 'Failure_Namespace_Invalid_Name';
	case 0x804D0002: return 'Failure_Mosaic_Invalid_Name';
	case 0x804E0003: return 'Failure_Namespace_Name_Id_Mismatch';
	case 0x804D0003: return 'Failure_Mosaic_Name_Id_Mismatch';
	case 0x804E0004: return 'Failure_Namespace_Expired';
	case 0x804D0004: return 'Failure_Mosaic_Expired';
	case 0x804E0005: return 'Failure_Namespace_Owner_Conflict';
	case 0x804D0005: return 'Failure_Mosaic_Owner_Conflict';
	case 0x804E0064: return 'Failure_Namespace_Invalid_Namespace_Type';
	case 0x804E0065: return 'Failure_Namespace_Root_Name_Reserved';
	case 0x804E0066: return 'Failure_Namespace_Too_Deep';
	case 0x804E0067: return 'Failure_Namespace_Parent_Unknown';
	case 0x804E0096: return 'Failure_Namespace_Already_Exists';
	case 0x804E0097: return 'Failure_Namespace_Already_Active';
	case 0x804E0098: return 'Failure_Namespace_Eternal_After_Nemesis_Block';
	case 0x804D0064: return 'Failure_Mosaic_Parent_Id_Conflict';
	case 0x804D0065: return 'Failure_Mosaic_Invalid_Property';
	case 0x804D0066: return 'Failure_Mosaic_Invalid_Flags';
	case 0x804D0067: return 'Failure_Mosaic_Invalid_Divisibility';
	case 0x804D0068: return 'Failure_Mosaic_Invalid_Supply_Change_Direction';
	case 0x804D0069: return 'Failure_Mosaic_Invalid_Supply_Change_Amount';
	case 0x804D006A: return 'Failure_Mosaic_Name_Reserved';
	case 0x804D0096: return 'Failure_Mosaic_Modification_Disallowed';
	case 0x804D0097: return 'Failure_Mosaic_Modification_No_Changes';
	case 0x804D00A1: return 'Failure_Mosaic_Supply_Immutable';
	case 0x804D00A2: return 'Failure_Mosaic_Supply_Negative';
	case 0x804D00A3: return 'Failure_Mosaic_Supply_Exceeded';
	case 0x804D00A4: return 'Failure_Mosaic_Non_Transferable';
	case 0x80540006: return 'Failure_Transfer_Message_Too_Large';
	case 0x805400C8: return 'Failure_Transfer_Out_Of_Order_Mosaics';
	case 0x80FF0066: return 'Failure_Chain_Unlinked';
	case 0x80FF0068: return 'Failure_Chain_Block_Not_Hit';
	case 0x80FE0001: return 'Failure_Consumer_Empty_Input';
	case 0x80FE1001: return 'Failure_Consumer_Block_Transactions_Hash_Mismatch';
	case 0x81FE1002: return 'Failure_Consumer_Hash_In_Recency_Cache';
	case 0x80FE2001: return 'Failure_Consumer_Remote_Chain_Too_Many_Blocks';
	case 0x80FE2002: return 'Failure_Consumer_Remote_Chain_Improper_Link';
	case 0x80FE2003: return 'Failure_Consumer_Remote_Chain_Duplicate_Transactions';
	case 0x80FE3001: return 'Failure_Consumer_Remote_Chain_Unlinked';
	case 0x80FE3002: return 'Failure_Consumer_Remote_Chain_Mismatched_Difficulties';
	case 0x80FE3003: return 'Failure_Consumer_Remote_Chain_Score_Not_Better';
	case 0x80FE3004: return 'Failure_Consumer_Remote_Chain_Too_Far_Behind';
	case 0x80450101: return 'Failure_Extension_Partial_Transaction_Cache_Prune';
	case 0x80450102: return 'Failure_Extension_Partial_Transaction_Dependency_Removed';
	default: return undefined;
	}
};

const status = {
	/**
	 * Converts a status code to a string.
	 * @param {numeric} code The status code.
	 * @returns {string} The string representation of the status code.
	 */
	toString: code => {
		const str = toStringInternal(code);
		if (undefined !== str)
			return str;

		let hexString = code.toString(16).toUpperCase();
		if (8 > hexString.length)
			hexString = '0'.repeat(8 - hexString.length) + hexString;

		return `unknown status 0x${hexString}`;
	}
};

module.exports = status;

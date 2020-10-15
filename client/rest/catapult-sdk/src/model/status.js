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

/** @module model/status */

/* istanbul ignore next */
const toStringInternal = code => {
	switch (code) {
	case 0x00000000: return 'Success';
	case 0x40000000: return 'Neutral';
	case 0x80000000: return 'Failure';
	case 0x80430001: return 'Failure_Core_Past_Deadline';
	case 0x80430002: return 'Failure_Core_Future_Deadline';
	case 0x80430003: return 'Failure_Core_Insufficient_Balance';
	case 0x80430004: return 'Failure_Core_Too_Many_Transactions';
	case 0x80430005: return 'Failure_Core_Nemesis_Account_Signed_After_Nemesis_Block';
	case 0x80430006: return 'Failure_Core_Wrong_Network';
	case 0x80430007: return 'Failure_Core_Invalid_Address';
	case 0x80430008: return 'Failure_Core_Invalid_Version';
	case 0x80430009: return 'Failure_Core_Invalid_Transaction_Fee';
	case 0x8043000A: return 'Failure_Core_Block_Harvester_Ineligible';
	case 0x8043000B: return 'Failure_Core_Zero_Address';
	case 0x8043000C: return 'Failure_Core_Zero_Public_Key';
	case 0x8043000D: return 'Failure_Core_Nonzero_Internal_Padding';
	case 0x8043000E: return 'Failure_Core_Address_Collision';
	case 0x80430065: return 'Failure_Core_Invalid_Link_Action';
	case 0x80430066: return 'Failure_Core_Link_Already_Exists';
	case 0x80430067: return 'Failure_Core_Inconsistent_Unlink_Data';
	case 0x80430068: return 'Failure_Core_Invalid_Link_Range';
	case 0x80430069: return 'Failure_Core_Too_Many_Links';
	case 0x81490001: return 'Failure_Hash_Already_Exists';
	case 0x80530001: return 'Failure_Signature_Not_Verifiable';
	case 0x804C0001: return 'Failure_AccountLink_Link_Already_Exists';
	case 0x804C0002: return 'Failure_AccountLink_Inconsistent_Unlink_Data';
	case 0x804C0003: return 'Failure_AccountLink_Unknown_Link';
	case 0x804C0004: return 'Failure_AccountLink_Remote_Account_Ineligible';
	case 0x804C0005: return 'Failure_AccountLink_Remote_Account_Signer_Prohibited';
	case 0x804C0006: return 'Failure_AccountLink_Remote_Account_Participant_Prohibited';
	case 0x80410001: return 'Failure_Aggregate_Too_Many_Transactions';
	case 0x80410002: return 'Failure_Aggregate_No_Transactions';
	case 0x80410003: return 'Failure_Aggregate_Too_Many_Cosignatures';
	case 0x80410004: return 'Failure_Aggregate_Redundant_Cosignatures';
	case 0x80410005: return 'Failure_Aggregate_Ineligible_Cosignatories';
	case 0x80410006: return 'Failure_Aggregate_Missing_Cosignatures';
	case 0x80410007: return 'Failure_Aggregate_Transactions_Hash_Mismatch';
	case 0x80480001: return 'Failure_LockHash_Invalid_Mosaic_Id';
	case 0x80480002: return 'Failure_LockHash_Invalid_Mosaic_Amount';
	case 0x80480003: return 'Failure_LockHash_Hash_Already_Exists';
	case 0x80480004: return 'Failure_LockHash_Unknown_Hash';
	case 0x80480005: return 'Failure_LockHash_Inactive_Hash';
	case 0x80480006: return 'Failure_LockHash_Invalid_Duration';
	case 0x80520001: return 'Failure_LockSecret_Invalid_Hash_Algorithm';
	case 0x80520002: return 'Failure_LockSecret_Hash_Already_Exists';
	case 0x80520003: return 'Failure_LockSecret_Proof_Size_Out_Of_Bounds';
	case 0x80520004: return 'Failure_LockSecret_Secret_Mismatch';
	case 0x80520005: return 'Failure_LockSecret_Unknown_Composite_Key';
	case 0x80520006: return 'Failure_LockSecret_Inactive_Secret';
	case 0x80520007: return 'Failure_LockSecret_Hash_Algorithm_Mismatch';
	case 0x80520008: return 'Failure_LockSecret_Invalid_Duration';
	case 0x80440001: return 'Failure_Metadata_Value_Too_Small';
	case 0x80440002: return 'Failure_Metadata_Value_Too_Large';
	case 0x80440003: return 'Failure_Metadata_Value_Size_Delta_Too_Large';
	case 0x80440004: return 'Failure_Metadata_Value_Size_Delta_Mismatch';
	case 0x80440005: return 'Failure_Metadata_Value_Change_Irreversible';
	case 0x804D0001: return 'Failure_Mosaic_Invalid_Duration';
	case 0x804D0002: return 'Failure_Mosaic_Invalid_Name';
	case 0x804D0003: return 'Failure_Mosaic_Name_Id_Mismatch';
	case 0x804D0004: return 'Failure_Mosaic_Expired';
	case 0x804D0005: return 'Failure_Mosaic_Owner_Conflict';
	case 0x804D0006: return 'Failure_Mosaic_Id_Mismatch';
	case 0x804D0064: return 'Failure_Mosaic_Parent_Id_Conflict';
	case 0x804D0065: return 'Failure_Mosaic_Invalid_Property';
	case 0x804D0066: return 'Failure_Mosaic_Invalid_Flags';
	case 0x804D0067: return 'Failure_Mosaic_Invalid_Divisibility';
	case 0x804D0068: return 'Failure_Mosaic_Invalid_Supply_Change_Action';
	case 0x804D0069: return 'Failure_Mosaic_Invalid_Supply_Change_Amount';
	case 0x804D006A: return 'Failure_Mosaic_Invalid_Id';
	case 0x804D006B: return 'Failure_Mosaic_Modification_Disallowed';
	case 0x804D006C: return 'Failure_Mosaic_Modification_No_Changes';
	case 0x804D006D: return 'Failure_Mosaic_Supply_Immutable';
	case 0x804D006E: return 'Failure_Mosaic_Supply_Negative';
	case 0x804D006F: return 'Failure_Mosaic_Supply_Exceeded';
	case 0x804D0070: return 'Failure_Mosaic_Non_Transferable';
	case 0x804D0071: return 'Failure_Mosaic_Max_Mosaics_Exceeded';
	case 0x804D0072: return 'Failure_Mosaic_Required_Property_Flag_Unset';
	case 0x80550001: return 'Failure_Multisig_Account_In_Both_Sets';
	case 0x80550002: return 'Failure_Multisig_Multiple_Deletes';
	case 0x80550003: return 'Failure_Multisig_Redundant_Modification';
	case 0x80550004: return 'Failure_Multisig_Unknown_Multisig_Account';
	case 0x80550005: return 'Failure_Multisig_Not_A_Cosignatory';
	case 0x80550006: return 'Failure_Multisig_Already_A_Cosignatory';
	case 0x80550007: return 'Failure_Multisig_Min_Setting_Out_Of_Range';
	case 0x80550008: return 'Failure_Multisig_Min_Setting_Larger_Than_Num_Cosignatories';
	case 0x80550009: return 'Failure_Multisig_Invalid_Modification_Action';
	case 0x8055000A: return 'Failure_Multisig_Max_Cosigned_Accounts';
	case 0x8055000B: return 'Failure_Multisig_Max_Cosignatories';
	case 0x8055000C: return 'Failure_Multisig_Loop';
	case 0x8055000D: return 'Failure_Multisig_Max_Multisig_Depth';
	case 0x8055000E: return 'Failure_Multisig_Operation_Prohibited_By_Account';
	case 0x804E0001: return 'Failure_Namespace_Invalid_Duration';
	case 0x804E0002: return 'Failure_Namespace_Invalid_Name';
	case 0x804E0003: return 'Failure_Namespace_Name_Id_Mismatch';
	case 0x804E0004: return 'Failure_Namespace_Expired';
	case 0x804E0005: return 'Failure_Namespace_Owner_Conflict';
	case 0x804E0006: return 'Failure_Namespace_Id_Mismatch';
	case 0x804E0064: return 'Failure_Namespace_Invalid_Registration_Type';
	case 0x804E0065: return 'Failure_Namespace_Root_Name_Reserved';
	case 0x804E0066: return 'Failure_Namespace_Too_Deep';
	case 0x804E0067: return 'Failure_Namespace_Unknown_Parent';
	case 0x804E0068: return 'Failure_Namespace_Already_Exists';
	case 0x804E0069: return 'Failure_Namespace_Already_Active';
	case 0x804E006A: return 'Failure_Namespace_Eternal_After_Nemesis_Block';
	case 0x804E006B: return 'Failure_Namespace_Max_Children_Exceeded';
	case 0x804E006C: return 'Failure_Namespace_Alias_Invalid_Action';
	case 0x804E006D: return 'Failure_Namespace_Unknown';
	case 0x804E006E: return 'Failure_Namespace_Alias_Already_Exists';
	case 0x804E006F: return 'Failure_Namespace_Unknown_Alias';
	case 0x804E0070: return 'Failure_Namespace_Alias_Inconsistent_Unlink_Type';
	case 0x804E0071: return 'Failure_Namespace_Alias_Inconsistent_Unlink_Data';
	case 0x804E0072: return 'Failure_Namespace_Alias_Invalid_Address';
	case 0x80500001: return 'Failure_RestrictionAccount_Invalid_Restriction_Flags';
	case 0x80500002: return 'Failure_RestrictionAccount_Invalid_Modification_Action';
	case 0x80500003: return 'Failure_RestrictionAccount_Invalid_Modification_Address';
	case 0x80500004: return 'Failure_RestrictionAccount_Modification_Operation_Type_Incompatible';
	case 0x80500005: return 'Failure_RestrictionAccount_Redundant_Modification';
	case 0x80500006: return 'Failure_RestrictionAccount_Invalid_Modification';
	case 0x80500007: return 'Failure_RestrictionAccount_Modification_Count_Exceeded';
	case 0x80500008: return 'Failure_RestrictionAccount_No_Modifications';
	case 0x80500009: return 'Failure_RestrictionAccount_Values_Count_Exceeded';
	case 0x8050000A: return 'Failure_RestrictionAccount_Invalid_Value';
	case 0x8050000B: return 'Failure_RestrictionAccount_Address_Interaction_Prohibited';
	case 0x8050000C: return 'Failure_RestrictionAccount_Mosaic_Transfer_Prohibited';
	case 0x8050000D: return 'Failure_RestrictionAccount_Operation_Type_Prohibited';
	case 0x80510001: return 'Failure_RestrictionMosaic_Invalid_Restriction_Type';
	case 0x80510002: return 'Failure_RestrictionMosaic_Previous_Value_Mismatch';
	case 0x80510003: return 'Failure_RestrictionMosaic_Previous_Value_Must_Be_Zero';
	case 0x80510004: return 'Failure_RestrictionMosaic_Max_Restrictions_Exceeded';
	case 0x80510005: return 'Failure_RestrictionMosaic_Cannot_Delete_Nonexistent_Restriction';
	case 0x80510006: return 'Failure_RestrictionMosaic_Unknown_Global_Restriction';
	case 0x80510007: return 'Failure_RestrictionMosaic_Invalid_Global_Restriction';
	case 0x80510008: return 'Failure_RestrictionMosaic_Account_Unauthorized';
	case 0x80540001: return 'Failure_Transfer_Message_Too_Large';
	case 0x80540002: return 'Failure_Transfer_Out_Of_Order_Mosaics';
	case 0x80FF0001: return 'Failure_Chain_Unlinked';
	case 0x80FF0002: return 'Failure_Chain_Block_Not_Hit';
	case 0x80FF0003: return 'Failure_Chain_Block_Inconsistent_State_Hash';
	case 0x80FF0004: return 'Failure_Chain_Block_Inconsistent_Receipts_Hash';
	case 0x80FF0005: return 'Failure_Chain_Unconfirmed_Cache_Too_Full';
	case 0x80FE0001: return 'Failure_Consumer_Empty_Input';
	case 0x80FE0002: return 'Failure_Consumer_Block_Transactions_Hash_Mismatch';
	case 0x41FE0003: return 'Neutral_Consumer_Hash_In_Recency_Cache';
	case 0x80FE0004: return 'Failure_Consumer_Remote_Chain_Too_Many_Blocks';
	case 0x80FE0005: return 'Failure_Consumer_Remote_Chain_Improper_Link';
	case 0x80FE0006: return 'Failure_Consumer_Remote_Chain_Duplicate_Transactions';
	case 0x80FE0007: return 'Failure_Consumer_Remote_Chain_Unlinked';
	case 0x80FE0008: return 'Failure_Consumer_Remote_Chain_Difficulties_Mismatch';
	case 0x80FE0009: return 'Failure_Consumer_Remote_Chain_Score_Not_Better';
	case 0x80FE000A: return 'Failure_Consumer_Remote_Chain_Too_Far_Behind';
	case 0x80FE000B: return 'Failure_Consumer_Remote_Chain_Too_Far_In_Future';
	case 0x80FE000C: return 'Failure_Consumer_Batch_Signature_Not_Verifiable';
	case 0x80450001: return 'Failure_Extension_Partial_Transaction_Cache_Prune';
	case 0x80450002: return 'Failure_Extension_Partial_Transaction_Dependency_Removed';
	case 0x80450003: return 'Failure_Extension_Read_Rate_Limit_Exceeded';
	default: return undefined;
	}
};

const status = {
	/**
	 * Converts a status code to a string.
	 * @param {numeric} code Status code.
	 * @returns {string} String representation of the status code.
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

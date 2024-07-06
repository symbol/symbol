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

import * as merkle from './crypto/merkleAuditProof.js';
import ModelType from './model/ModelType.js';
import NamespaceAliasType from './model/NamespaceAliasType.js';
import idReducer from './model/idReducer.js';
import status from './model/status.js';
import { PacketType, StatePathPacketTypes } from './packet/PacketType.js';
import packetHeader from './packet/header.js';
import BinaryParser from './parser/BinaryParser.js';
import PacketParser from './parser/PacketParser.js';
import catapultModelSystem from './plugins/catapultModelSystem.js';
import CachedFileLoader from './utils/CachedFileLoader.js';
import SchemaType from './utils/SchemaType.js';
import formattingUtils from './utils/formattingUtils.js';
import future from './utils/future.js';
import objects from './utils/objects.js';
import schemaFormatter from './utils/schemaFormatter.js';

export default {
	crypto: {
		merkle
	},
	model: {
		idReducer,
		ModelType,
		NamespaceAliasType,
		status
	},
	packet: {
		header: packetHeader,
		PacketType,
		StatePathPacketTypes
	},
	parser: {
		BinaryParser,
		PacketParser
	},
	plugins: {
		catapultModelSystem
	},
	utils: {
		CachedFileLoader,
		SchemaType,
		formattingUtils,
		future,
		objects,
		schemaFormatter
	}
};

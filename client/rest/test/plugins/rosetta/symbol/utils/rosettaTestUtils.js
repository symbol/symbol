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

import AccountIdentifier from '../../../../../src/plugins/rosetta/openApi/model/AccountIdentifier.js';
import Amount from '../../../../../src/plugins/rosetta/openApi/model/Amount.js';
import Currency from '../../../../../src/plugins/rosetta/openApi/model/Currency.js';
import Operation from '../../../../../src/plugins/rosetta/openApi/model/Operation.js';
import OperationIdentifier from '../../../../../src/plugins/rosetta/openApi/model/OperationIdentifier.js';
import CatapultProxy from '../../../../../src/plugins/rosetta/symbol/CatapultProxy.js';
import {
	BasicFetchStubHelper,
	assertRosettaErrorRaisedBasicWithRegister,
	assertRosettaSuccessBasicWithRegister
} from '../../utils/rosettaTestUtils.js';
import { PrivateKey } from 'symbol-sdk';
import { KeyPair } from 'symbol-sdk/symbol';

// region constants

export const createRosettaAggregateSignerKeyPair = () =>
	new KeyPair(new PrivateKey('C7CAC863460EBB139F0B61778A285F31B8E8548B3D2354832946C880735C1F70'));

// endregion

// region FetchStubHelper

export const FetchStubHelper = {
	...BasicFetchStubHelper,

	stubCatapultProxyCacheFill: () => {
		FetchStubHelper.stubPost('node/info', true, {});
		FetchStubHelper.stubPost('network/properties', true, {});
		FetchStubHelper.stubPost('blocks/1', true, {});
	},

	stubMosaicResolution: (mosaicIdHexString, name, divisibility) => {
		FetchStubHelper.stubPost(`mosaics/${mosaicIdHexString}`, true, { mosaic: { divisibility } });
		FetchStubHelper.stubPost('namespaces/mosaic/names', true, { mosaicNames: [{ names: [name] }] }, {
			method: 'POST',
			body: JSON.stringify({ mosaicIds: [mosaicIdHexString] }),
			headers: { 'Content-Type': 'application/json' }
		});
	}
};

// endregion

// region RosettaObjectFactory

export const RosettaObjectFactory = {
	createRosettaNetworkIdentifier: () => ({
		blockchain: 'Symbol',
		network: 'testnet'
	}),

	createRosettaPublicKey: hexBytes => ({
		hex_bytes: hexBytes,
		curve_type: 'edwards25519'
	})
};

// endregion

// region RosettaOperationFactory

export const RosettaOperationFactory = {
	createCurrency: (currencyName, currencyDecimals, mosaicId) => {
		const currency = new Currency(currencyName, currencyDecimals);
		if (mosaicId)
			currency.metadata = { id: mosaicId };

		return currency;
	},

	createTransferOperation: (index, address, amount, currencyName, currencyDecimals, mosaicId = undefined) => {
		const currency = RosettaOperationFactory.createCurrency(currencyName, currencyDecimals, mosaicId);

		const operation = new Operation(new OperationIdentifier(index), 'transfer');
		operation.account = new AccountIdentifier(address);
		operation.amount = new Amount(amount, currency);
		return operation;
	},

	createMultisigOperation: (index, address, metadata) => {
		const operation = new Operation(new OperationIdentifier(index), 'multisig');
		operation.account = new AccountIdentifier(address);
		operation.metadata = {
			addressAdditions: [],
			addressDeletions: [],
			...metadata
		};
		return operation;
	},

	createCosignOperation: (index, address) => {
		const operation = new Operation(new OperationIdentifier(index), 'cosign');
		operation.account = new AccountIdentifier(address);
		return operation;
	},

	setOperationStatus: operation => {
		operation.status = 'success';
		return operation;
	}
};

// endregion

// region asserts

const createRegisterRoutes = routes => server => {
	routes.register(server, {}, {
		config: {
			network: { name: 'testnet' },
			rosetta: { aggregateSignerPrivateKey: createRosettaAggregateSignerKeyPair().privateKey.toString() }
		},
		proxy: new CatapultProxy('http://localhost:3456')
	});
};

export const assertRosettaErrorRaisedBasicWithRoutes = async (routes, ...args) => assertRosettaErrorRaisedBasicWithRegister(
	createRegisterRoutes(routes),
	...args
);

export const assertRosettaSuccessBasicWithRoutes = async (routes, ...args) => assertRosettaSuccessBasicWithRegister(
	createRegisterRoutes(routes),
	...args
);

// endregion

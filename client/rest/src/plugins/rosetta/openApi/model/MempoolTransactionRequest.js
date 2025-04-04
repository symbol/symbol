/**
 * Rosetta
 * Build Once. Integrate Your Blockchain Everywhere.
 *
 * The version of the OpenAPI document: 1.4.13
 * 
 *
 * NOTE: This class is auto generated by OpenAPI Generator (https://openapi-generator.tech).
 * https://openapi-generator.tech
 * Do not edit the class manually.
 *
 */

import ApiClient from '../ApiClient.js';
import NetworkIdentifier from './NetworkIdentifier.js';
import TransactionIdentifier from './TransactionIdentifier.js';

/**
 * The MempoolTransactionRequest model module.
 * @module model/MempoolTransactionRequest
 * @version 1.4.13
 */
class MempoolTransactionRequest {
	/**
	 * Constructs a new <code>MempoolTransactionRequest</code>.
	 * A MempoolTransactionRequest is utilized to retrieve a transaction from the mempool.
	 * @alias module:model/MempoolTransactionRequest
	 * @param networkIdentifier {module:model/NetworkIdentifier}
	 * @param transactionIdentifier {module:model/TransactionIdentifier}
	 */
	constructor(networkIdentifier, transactionIdentifier) {

		MempoolTransactionRequest.initialize(this, networkIdentifier, transactionIdentifier);
	}

	/**
	 * Initializes the fields of this object.
	 * This method is used by the constructors of any subclasses, in order to implement multiple inheritance (mix-ins).
	 * Only for internal use.
	 */
	static initialize(obj, networkIdentifier, transactionIdentifier) {
		obj['network_identifier'] = networkIdentifier;
		obj['transaction_identifier'] = transactionIdentifier;
	}

	/**
	 * Constructs a <code>MempoolTransactionRequest</code> from a plain JavaScript object, optionally creating a new instance.
	 * Copies all relevant properties from <code>data</code> to <code>obj</code> if supplied or a new instance if not.
	 * @param {Object} data The plain JavaScript object bearing properties of interest.
	 * @param {module:model/MempoolTransactionRequest} obj Optional instance to populate.
	 * @return {module:model/MempoolTransactionRequest} The populated <code>MempoolTransactionRequest</code> instance.
	 */
	static constructFromObject(data, obj) {
		if (data) {
			obj = obj || new MempoolTransactionRequest();

			if (data.hasOwnProperty('network_identifier')) {
				obj['network_identifier'] = NetworkIdentifier.constructFromObject(data['network_identifier']);
			}
			if (data.hasOwnProperty('transaction_identifier')) {
				obj['transaction_identifier'] = TransactionIdentifier.constructFromObject(data['transaction_identifier']);
			}
		}
		return obj;
	}

	/**
	 * Validates the JSON data with respect to <code>MempoolTransactionRequest</code>.
	 * @param {Object} data The plain JavaScript object bearing properties of interest.
	 * @return {boolean} to indicate whether the JSON data is valid with respect to <code>MempoolTransactionRequest</code>.
	 */
	static validateJSON(data) {
		// check to make sure all required properties are present in the JSON string
		for (const property of MempoolTransactionRequest.RequiredProperties) {
			if (!data.hasOwnProperty(property)) {
				throw new Error("The required field `" + property + "` is not found in the JSON data: " + JSON.stringify(data));
			}
		}
		// validate the optional field `network_identifier`
		if (data['network_identifier']) { // data not null
		  NetworkIdentifier.validateJSON(data['network_identifier']);
		}
		// validate the optional field `transaction_identifier`
		if (data['transaction_identifier']) { // data not null
		  TransactionIdentifier.validateJSON(data['transaction_identifier']);
		}

		return true;
	}


}

MempoolTransactionRequest.RequiredProperties = ["network_identifier", "transaction_identifier"];

/**
 * @member {module:model/NetworkIdentifier} network_identifier
 */
MempoolTransactionRequest.prototype['network_identifier'] = undefined;

/**
 * @member {module:model/TransactionIdentifier} transaction_identifier
 */
MempoolTransactionRequest.prototype['transaction_identifier'] = undefined;






export default MempoolTransactionRequest;

